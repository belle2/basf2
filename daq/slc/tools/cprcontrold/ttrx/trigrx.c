/* ---------------------------------------------------------------------- *\
   trigrx.c

   generate trigger / read fifo

   Mikihiko Nakao, KEK IPNS

   version
   2003110400  working version
   2003110500  -mask option added
   2003110600  check next fifo at bad fifo read
   2004090100  TT-RX v4
   2004090700  -noreset
   2004110100  -out
   2005021501  ttrxlib version
   2005040700  for rx 0.8
   2005041200  local clk, fifo empty check when tlast=0
   2005041300  -localbusy, exclusive with mask
   2005041301  bad fifo tracing
   2005052000  better args handling
   2005072301  backport from trigio.c
   2005072500  nev0first
   2005072700  xmask fix
   2005081600  clear
   2006041300  -delay
\* ---------------------------------------------------------------------- */

#define VERSION 2006041300

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ttrx.h"

#define B(r,n)     (((r)>>(n))&1)
#define W(r,n,a,b) ((((r)>>(n))&1)?(a):(b))
#define R(r,n,w)   (((r)>>(n))&((1<<w)-1))

static int xquiet = 0;
static int xnoread = 0;
static int xnofifo = 0;
static int xnocheck = 0;
static int xnoreset = 1;
static int xignorebusy = 0;
static int xstdbusy = 1;
static int xextclk  = 1;
static int xdelay  = 0;

static int xmask = 0x10;

/* ---------------------------------------------------------------------- *\
   init_trigrx
\* ---------------------------------------------------------------------- */
int
init_trigrx(ttrx_t *ttrx, int trgsel, double trgrate, int count)
{
  unsigned int csr, trig, tlast, rate, id, ver, pldid, mask;
  int rate_a = (int)(log(42333000.0/trgrate/512)/log(2.0)) & 0xff;
  int rate_n = (int)(42333000.0/trgrate/pow(2,rate_a) - 0.5) & 0xffffff;

  if (! xnoreset) {
    write_ttrx(ttrx, TTRX_PRGM, 0x0e);    /* reset */
  }
  
  trig = (3 << 24) | (1 << 31);      /* disable trigger before FIFO reset */
  if (trgsel == 3) trig |= 0xffffff;
  write_ttrx(ttrx, TTRX_TRIG, trig); /* and disable busy during setup */
  if (trgsel == 3) return -1;

  write_ttrx(ttrx, TTRX_MASK, xmask);
  
  csr = (1<<11) | (1<<7);        /* clear FIFO and exbsy */
  if (!xstdbusy) csr |= (1<<12); /* use test busy (entestbsy) */
  if (xnofifo)   csr |= (3<<5);  /* don't stop at TT-RX FIFO half/full */
  if (xextclk)   csr |= (1<<26);

  write_ttrx(ttrx, TTRX_CSR, csr);

  pldid = read_ttrx(ttrx, TTRX_PID) & 0x0f;
  id   = read_ttrx(ttrx, TTRX_ID);
  ver  = read_ttrx(ttrx, TTRX_VER);
  csr  = read_ttrx(ttrx, TTRX_CSR);

  if (pldid == 0) {
    printf("PLD id=0? broken TT-RX?\n");
    return 0;
  }
  if ((id|ver|csr) == 0) {
    printf("no FPGA data, jumper pin forgotten between C_EN--GND?\n");
    return 0;
  }

  if (! xnofifo && B(csr,0) == 0) {
    printf("FIFO is not cleared.\n");
    return 0;
  }
  if (! xignorebusy && B(csr,9)) {
    printf("BUSY before any trigger.\n");
    return 0;
  }
  if (! xignorebusy && B(csr,11)) {
    printf("EXTRA busy before any trigger.\n");
    return 0;
  }

  write_ttrx(ttrx, TTRX_RATE, (rate_a << 24) | rate_n);
  rate = read_ttrx(ttrx, TTRX_RATE);
  if (rate != ((rate_a << 24) | rate_n)) {
    printf("can't readback rate register? rate=%08x a=%02x n=%06x\n",
	   rate, rate_a, rate_n);
  }

  write_ttrx(ttrx, TTRX_DEPTH, (xdelay&0xff)<<8);
  trig = ((trgsel & 3) << 24) | (count & 0xffffff);
  if (xignorebusy)    trig |= (1<<30);
  trig |= (1<<31);
  write_ttrx(ttrx, TTRX_TRIG, trig);
  trig &= ~(1<<31);
  write_ttrx(ttrx, TTRX_TRIG, trig);
  trig  = read_ttrx(ttrx, TTRX_TRIG);
  tlast = read_ttrx(ttrx, TTRX_TLAST);
  mask  = read_ttrx(ttrx, TTRX_MASK);
  if (! xquiet) {
    printf("csr=%08x trig=%08x tlast=%08x rate=%08x mask=%08x\n",
	   csr, trig, tlast, rate, mask);
  }

  return 1;
}
/* ---------------------------------------------------------------------- *\
   loop_trigrx
\* ---------------------------------------------------------------------- */
void
loop_trigrx(ttrx_t *ttrx, int count)
{
  int i, ret;
  int nevall = 0;
  int nev = 0;
  unsigned csr, trig, tlast;
  time_t t0, t1, tnext;
  struct timeval start, end;
  double dt;
  unsigned int fifo[2], fifonext[2];
  int nfifoerr = 0;
  int rfcount = 0;

  gettimeofday(&start, NULL);
  time(&t0);
  tnext = t0 + 2;
  fifo[0] = fifo[1] = -1;

  while (1) {
    trig  = read_ttrx(ttrx, TTRX_TRIG);
    tlast = read_ttrx(ttrx, TTRX_TLAST);

    for (i=0; i<1; i++) {
      csr = read_ttrx(ttrx, TTRX_CSR);
      while ((csr & 1) == 0) { /* while fifo not empty (bit-0 = fifo empty) */
        if ((ret = readfifo_ttrx(ttrx, fifo)) < 0) {
          printf("can't read fifo (%d)\n", ret);
          break;
        }

        nevall++;
        nev++;
        if (fifo[0] != nevall-1) {
          if (xnocheck) {
            nfifoerr++;
          } else {
            write_ttrx(ttrx, TTRX_INTR, 4); /* to trigger scope */
            readfifo_ttrx(ttrx, fifonext);
            printf("bad fifo1=%d (%d expected %d next) depth=%d tag=%d last=%d\n",
                   fifo[0], nevall-1, fifonext[0],
		   read_ttrx(ttrx, TTRX_DEPTH),
		   read_ttrx(ttrx, TTRX_TAG),
		   read_ttrx(ttrx, TTRX_TLAST));
            break;
          }
        }
        csr = read_ttrx(ttrx, TTRX_CSR); /* extra busy check */
        if (csr & (1<<11)) {
          printf("extra busy!\n");
          break;
        }
      }
      if (!xnocheck && nevall>0 && fifo[0] != nevall-1) break; /* bad fifo */
      if (csr & (1<<11)) break;                /* extra busy */
    }

    tlast = read_ttrx(ttrx, TTRX_TLAST) & 0xffffff;
    csr   = read_ttrx(ttrx, TTRX_CSR);
    if (tlast == 0 && (csr & 1) == 1) break; /* all done and fifo empty */

    time(&t1);
    if (t1 >= tnext) {
      tnext = t1 + 2;
      t1 -= t0;
      gettimeofday(&end, NULL);
      dt = end.tv_sec - start.tv_sec;
      dt += (end.tv_usec - start.tv_usec) / 1e6;
      if (! xquiet) {
	static int nev0first = 0;
	if (nev || nev0first)
	  printf("event %d fifo1=%08x fifo2=%08x rate=%3.1f Hz t=%d",
		 nevall, fifo[0], fifo[1], dt>0?nevall/dt:0, t1);
	if (nev == 0 && nev0first) printf(" (no event!)");
	if (nev || nev0first) printf("\n");
	nev0first = nev;
	fflush(stdout);
      }
      nev = 0;
    }

    if (i != 1) break;
  }

  gettimeofday(&end, NULL);
  dt = end.tv_sec - start.tv_sec;
  dt += (end.tv_usec - start.tv_usec) / 1e6;

  if (xquiet) {
    printf("done count=%d nevall=%d rate=%3.1e Hz t=%1.0f\n",
	   count, nevall, dt>0?nevall/dt:0, dt);
  } else {
    printf("done count=%d nevall=%d rate=%3.1f Hz t=%4.2f\n",
	   count, nevall, dt>0?nevall/dt:0, dt);
  }

  printf("     fifo1=%d tag=%d last=%d depth=%d set=%d fifoerr=%d\n",
         fifo[0],
	 read_ttrx(ttrx, TTRX_TAG),
	 read_ttrx(ttrx, TTRX_TLAST),
	 read_ttrx(ttrx, TTRX_DEPTH) & 0xff,
	 read_ttrx(ttrx, TTRX_TRIG) & 0xffffff,
	 nfifoerr);
}
/* ---------------------------------------------------------------------- *\
   usage
\* ---------------------------------------------------------------------- */
static void
usage()
{
  printf("usage: trigrx [options] <type> [<rate-in-Hz> [<count>]]\n");
  printf("options:\n");
  printf(" -<n>          select <n>-th TT-RX (<n>=0,1,2...9)\n");
  printf(" -quiet        be silent\n");
  printf(" -fifo=no      no fifo fill and read\n");
  printf(" -fifo=noread  fill fifo but not read\n");
  printf(" -fifo=nocheck fill/read fifo but no check\n");
  printf(" -localbusy    use dummy busy at every trigger (FINESSE busy is masked)\n");
  printf(" -localclk     use local sclk\n");
  printf(" -ignorebusy   no busy handshake (no fifo)\n");
  printf(" -noreset      no reset at the beginning\n");
  printf(" -mask=<m>     mask (ignore busy from) some FINESSE\n");
  printf("               <m>: 4-bit mask, e.g. <m>=6 masks FINESSE B,C\n");
  printf(" -delay=<l>    <l>: trigger delay (unit of 24ns), 0-255 => 0-6us\n");
  printf(" <type>        one of random / pulse / ext / clear\n");
  printf(" <count>       -1(default) for free-run mode\n");
  printf("\n");
  printf("example:\n");
  printf("  ./trigrx random 10000 1000000\n");
  printf("  (to run random (about) 10 kHz for about 100 seconds)\n");
  printf("\n");
}
/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
  double rate;
  int count = 0xffffff; /* free-run mode */
  int trgsel = -1;      /* to trap args error */
  int oflag = TTRX_RDWR|TTRX_USEFIFO;
  int rxid = 0;
  ttrx_t *ttrx = 0;
  
  printf("trigrx version %d\n", VERSION);

  while (argc > 1 && argv[1][0] == '-') {
    if (strcmp(argv[1], "-m") == 0) {
      oflag |= TTRX_MMAPMODE;
    } else if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "-debug") == 0) {
      debug_ttrx(1);
    } else if (strcmp(argv[1], "-quiet") == 0) {
      xquiet = 1;
    } else if (strcmp(argv[1], "-fifo=no") == 0) {
      xnofifo = 1;
    } else if (strcmp(argv[1], "-fifo=noread") == 0) {
      xnoread = 1;
    } else if (strcmp(argv[1], "-fifo=nocheck") == 0) {
      xnocheck = 1;
    } else if (strcmp(argv[1], "-localclk") == 0) {
      xextclk = 0;
    } else if (strcmp(argv[1], "-localbusy") == 0) {
      xstdbusy = 0;
      xmask = 15;
    } else if (strcmp(argv[1], "-noreset") == 0) {
      xnoreset = 1;
    } else if (strcmp(argv[1], "-ignorebusy") == 0) {
      xignorebusy = 1;
      xnofifo = 1;
    } else if (strncmp(argv[1], "-mask=", 6) == 0) {
      xmask = (strtoul(&argv[1][6], 0, 16) & 15) | 16;
      if (xmask == 31) {
         printf("use -localbusy option\n");
         return 1;
      } else if (xstdbusy == 0) {
         printf("can't set mask with -localbusy option\n");
         return 1;
      }
    } else if (strncmp(argv[1], "-delay=", 7) == 0) {
      xdelay = strtoul(&argv[1][7], 0, 0) & 255;
    } else if (argv[1][0] == '-' && isdigit(argv[1][1])) {
      rxid = argv[1][1] - '0';
    } else {
      argc = 0;
    }
    argc--, argv++;
  }
  if (argc > 1) {
    if (strcmp(argv[1], "random") == 0)
      trgsel = 1;
    else if (strcmp(argv[1], "pulse") == 0)
      trgsel = 2;
    else if (strcmp(argv[1], "ext") == 0) {
      trgsel = 0;
      rate = 0; /* dummy */
    } else if (strcmp(argv[1], "clear") == 0 || strcmp(argv[1], "stop") == 0) {
      trgsel = 3;
      rate = 0; /* dummy */
    }
    argc--, argv++;
  }
  
  if (argc >= 2) rate  = atof(argv[1]);
  if (argc >= 3) count = strtoul(argv[2], 0, 0) & 0xffffff;

  if (trgsel < 0 || rate < 0) {
    usage();
    return 1;
  }

  
  if (! (ttrx = open_ttrx(rxid, oflag))) {
    perror("open_ttrx");
    printf("TT-RX is not found.\n");
    exit(1);
  }

  if (init_trigrx(ttrx, trgsel, rate, count) < 0)
    return 1;

  if (xnofifo || xnoread) return 0;

  loop_trigrx(ttrx, count);

  return 0;
}
