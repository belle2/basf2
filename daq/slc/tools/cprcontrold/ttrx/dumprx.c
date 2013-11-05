/* ---------------------------------------------------------------------- *\
   dumprx.c

   PCI register dump for the TT-RX module

   Mikihiko Nakao, KEK IPNS

   2003061200  fix fifoful display
   2003061300  version display is added
   2003061901  nwff bit was wrong
   2003120200  new get_rx, new name (was dump-ttrx.c)
   2003120400  ttrx.h
   2004090100  v4
   2004091301  ttest tstat tsav tsav2 tng32
   2005021000  user mode
   2005021500  ttrxlib version
   2005040700  for rx 0.8
   2005041200  for rx 0.8r12
   2005072300  backport from tt-io
   2005072701  mask, fifoful
   2005081300  dcmd/ucmd, etc
   2005081600  serial
   2005082400  trig mode fix
   2006012500  tag
   2006012600  dcmd
\* ---------------------------------------------------------------------- */

#define VERSION 2006012600

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ttrx.h"

#define B(r,n)     (((r)>>(n))&1)
#define W(r,n,a,b) ((((r)>>(n))&1)?(a):(b))
#define R(r,n,w)   (((r)>>(n))&((1<<w)-1))

/* ---------------------------------------------------------------------- *\
   regdump
\* ---------------------------------------------------------------------- */
void
regdump(ttrx_t *ttrx, int readfifo, int summaryonly)
{
  unsigned int pid     = read_ttrx(ttrx, TTRX_PID)   & 0xf;
  unsigned int pver    = read_ttrx(ttrx, TTRX_PVER)  & 0xf;
  unsigned int prgm    = read_ttrx(ttrx, TTRX_PRGM)  & 0xf;
  unsigned int sclk    = read_ttrx(ttrx, TTRX_SCLK)  & 0xf;
  unsigned int ser1    = read_ttrx(ttrx, TTRX_SER1)  & 0xf;
  unsigned int ser2    = read_ttrx(ttrx, TTRX_SER2)  & 0xf;
  unsigned int ser3    = read_ttrx(ttrx, TTRX_SER3)  & 0xf;

  unsigned int id      = read_ttrx(ttrx, TTRX_ID);
  unsigned int ver     = read_ttrx(ttrx, TTRX_VER);
  unsigned int csr     = read_ttrx(ttrx, TTRX_CSR);
  unsigned int clk     = read_ttrx(ttrx, TTRX_CLK);
  unsigned int intr    = read_ttrx(ttrx, TTRX_INTR);
  unsigned int mask    = read_ttrx(ttrx, TTRX_MASK);

  unsigned int trig    = read_ttrx(ttrx, TTRX_TRIG);
  unsigned int tlast   = read_ttrx(ttrx, TTRX_TLAST);
  unsigned int rate    = read_ttrx(ttrx, TTRX_RATE);
  unsigned int tag     = read_ttrx(ttrx, TTRX_TAG);
  unsigned int tagr    = read_ttrx(ttrx, TTRX_TAGR);
  unsigned int depth   = read_ttrx(ttrx, TTRX_DEPTH);

  unsigned int ustat   = read_ttrx(ttrx, TTRX_USTAT);
  unsigned int uerrs   = read_ttrx(ttrx, TTRX_UERRS);
  unsigned int ucmd    = read_ttrx(ttrx, TTRX_UCMD);
  unsigned int uchk    = read_ttrx(ttrx, TTRX_UCHK);
  unsigned int uraw    = read_ttrx(ttrx, TTRX_URAW);

  unsigned int ext     = read_ttrx(ttrx, TTRX_EXT);

  char *trigsels[] = { "std", "random", "pulse", "none" };
  char *usels[]    = { "std", "incr", "thru", "usr" };

  char buf[256];
  double freq;
  int serial = ser1 + ser2 * 16 + ser3 * 256;
  int showver = 0;

  if (! summaryonly) {
    char *pidstr;
    switch (pid) {
    case 2: pidstr = "v3"; break;
    case 3: pidstr = "v4 prototype"; break;
    case 4: pidstr = "v4"; break;
    default: pidstr = "v?";
    }

    printf("--------------------------------------");
    printf("--------------------------------------\n");
    printf("dumprx %d --- ", VERSION);
    printf("TT-RX %s serial #%03d CPLD v0.%02d FPGA v%d.%02d rev %d\n",
	   pidstr, serial, pver, R(ver,16,12), R(ver,8,8), R(ver,0,8));
    printf("--------------------------------------");
    printf("--------------------------------------\n");

    if (showver) {
      printf("000    ID=%01x %s\n", pid, pidstr);
      printf("010   VER=%01x (CPLD version 0.%02d)\n", pver, pver);
    }

    printf("020  PRGM=%08x done=%d init=%d m012=%d(%s)\n",
	   prgm, B(prgm,3), B(prgm,2), B(prgm,1), W(prgm,1,"pci","flash"));
    printf("100    ID=%08x %s\n", id,
	   id==0x58525454?"OK":(id==0x058545454?"OK (Tx mode)":"NG"));

    if (showver) {
      printf("110   VER=%08x TT-RX version %d FPGA version %d.%02d rev %02d\n",
	     ver, (ver>>28)&0xf, (ver>>16)&0xfff, (ver>>8)&0xff, ver&0xff);
    }

    printf("%03x   CSR=%08x ulock*=%d usync*=%d",
	   TTRX_CSR<<2, csr, B(csr,31), B(csr,29));

    printf(" uclkok=%d uclksel=%d noready=%d\n",
           B(csr,27), B(csr,26), B(csr,16));

    printf("%19sdtag=%d utag=%d misc=%d entest=%d exbsy=%d nobsy=%d bsy=%d ready=%d\n",
	   "", B(csr,15), B(csr,14), B(csr,13), B(csr,12),
	   B(csr,11), B(csr,10), B(csr,9), B(csr,8));
    printf("%19sfifoful=%d orun=%d full=%d(%sabled) half=%d(%sabled) empty=%d\n",
	   "", B(csr,7), B(csr,3), B(csr,2), W(csr,6,"dis","en"),
	   B(csr,1), W(csr,5,"dis","en"), B(csr,0));
    printf("%03x   CLK=%08x uclkcnt=%d(%s) lclk(%s) rclk=1/%d+%d\n",
	   TTRX_CLK<<2, clk,
	   R(clk,24,7), W(clk,31,"v","_"),
	   W(clk,0,"v","_"), R(clk,16,8), R(clk,8,8));
    printf("%19suedgeo=%d dedgeo=%d sclkneg=%d dclkneg=%d\n",
	   "", B(clk,7), B(clk,6), B(clk,5), B(clk,4));
    printf("%03x  INTR=%08x irq=%d\n",
	   TTRX_INTR<<2, intr, B(intr,0));
    printf("%03x  MASK=%08x nobsy=%1x exbsy=%1x bsy=%1x mask=%1x\n",
	   TTRX_MASK<<2, mask,
	   R(mask,24,8), R(mask,16,8), R(mask,8,8), R(mask,0,8));

    printf("200  TRIG=%08x disbusy=%d decg=%d trigsel=%s",
	   trig, (trig>>31)&1, (trig>>30)&1, trigsels[(trig>>24)&3]);
    if ((trig&0xffffff) == 0xffffff) {
      printf(" (free-run)\n");
    } else {
      printf(" n=%d\n", trig&0xffffff);
    }
    printf("210 TLAST=%08x", tlast);
    if ((tlast&0xffffff) == 0xffffff) {
      printf(" (free-run)\n");
    } else {
      printf(" n=%d\n", tlast&0xffffff);
    }
    printf("220  RATE=%08x refclk=%d Hz[28:24] rndset=%d[17:16] plsset=1/%d[9:0]\n",
	   rate,
	   42333000/(1<<((rate>>24)&31)),
	   (rate>>16)&3,
	   (rate&((1<<10)-1))+1);
    printf("230   TAG=%08x %d\n", tag, tag);
    printf("250 DEPTH=%08x\n", depth);
    if (readfifo) {
      unsigned int fifo1 = read_ttrx(ttrx, TTRX_FIFO1);
      unsigned int fifo2 = read_ttrx(ttrx, TTRX_FIFO2);
      printf("\n");
      printf("300   fifo1=%08x\n", fifo1);
      printf("304   fifo2=%08x\n", fifo2);
    }
    printf("%03x USTAT=%08x lock*=%d sync*=%d(%d/%d) parity=%d/%d(%d)",
	   TTRX_USTAT<<2, ustat,
	   B(ustat,31), B(ustat,30), B(ustat,29), B(ustat,28),
	   B(ustat,26), B(ustat,25), B(ustat,24));
    printf(" in=%02x(%02x) out=%02x\n",
	   R(ustat,16,8), R(ustat,8,8), R(ustat,0,8));
    printf("%03x UERRS=%08x #parity=%d #sync=%d\n",
	   TTRX_UERRS<<2, uerrs, R(uerrs,16,16), R(uerrs,0,16));
    printf("%03x  UCMD=%08x ret=%02x in=%02x cmd=%02x set=%02x\n",
	   TTRX_UCMD<<2, ucmd,
           R(ucmd,16,8), R(ucmd,8,8), R(ucmd,0,8), R(ucmd,24,8));
    printf("%03x  UCHK=%08x err=%d\n",
	   TTRX_UCHK<<2, uchk, R(uchk,0,24));
    printf("%03x  URAW=%08x usr=%03x(%s) out=%03x(%s) in=%03x\n",
	   TTRX_URAW<<2, uraw, R(uraw,20,10), W(uraw,31,"v","_"),
	   R(uraw,10,10), W(uraw,31,"_","v"), R(uraw,0,10));

    printf("700   EXT=%08x %d %d %d %d\n",
	   ext, (ext>>24)&0xff, (ext>>16)&0xff, (ext>>8)&0xff, ext&0xff);
    printf("--------------------------------------");
    printf("--------------------------------------\n");
  }

  if (1) {
    int trgmode = R(trig,24,2);
    char *trgstr;
    freq = 0;
    switch (trgmode) {
    case 2:
      trgstr = "PULSE";
      freq = 42333000.0/(1<<((rate>>24)&31))/(1+rate&((1<<10)-1));
      break;
    case 1:
      trgstr = "RANDOM";
      freq = 42333000.0/(1<<((rate>>24)&31))/512;
      break;
    case 0:
      trgstr = "EXT";
      break;
    case 3:
      trgstr = "NONE";
    }
    
    printf("Trigger=%s count=%d", trgstr, tag);
    
    if (freq > 0) printf(" freq=%3.1f Hz", freq);
    if ((trig & ((1<<24)-1)) < (1<<24)-1) {
      printf(" (out of %d)", trig & ((1<<24)-1));
    } else {
      printf(" (no limit)");
    }
    
    printf("  Status=");
    if (B(csr,8)) {
      printf("READY\n");
    } else {
      int rx_fifofull = 0;
      int msk   = R(mask,  0, 8);
      int bsy   = R(mask,  8, 8) & ~msk;
      int exbsy = R(mask, 16, 8) & ~msk;
      int nobsy = R(mask, 24, 8) & ~msk;
      
      printf("NOT-READY mask=%s%s%s%s%s",
	     W(mask,0,"A",""), W(mask,1,"B",""),
	     W(mask,2,"C",""), W(mask,3,"D",""), W(mask,4,"none",""));
      if (exbsy) {
	printf(" exbusy=%s%s%s%s%s",
	       W(exbsy,0,"A",""), W(exbsy,1,"B",""),
	       W(exbsy,2,"C",""), W(exbsy,3,"D",""), W(exbsy,4,"self",""));
      }
      if (nobsy) {
	printf(" nobusy=%s%s%s%s%s",
	       W(nobsy,0,"A",""), W(nobsy,1,"B",""),
	       W(nobsy,2,"C",""), W(nobsy,3,"D",""), W(nobsy,4,"self",""));
      }
      if (bsy) {
	printf(" busy=%s%s%s%s%s",
	       W(bsy,0,"A",""), W(bsy,1,"B",""),
	       W(bsy,2,"C",""), W(bsy,3,"D",""), W(bsy,4,"self",""));
      }

      if (B(csr,2) && !B(csr,6)) rx_fifofull++; /* full */
      if (B(csr,1) && !B(csr,5)) rx_fifofull++; /* half */
      
      if (rx_fifofull) {
	printf(" fifo-full");
      }

      if ((tlast & 0x00ffffff) == 0) {
	printf(" trig-limit");
      }
      printf("\n");
    }
  }

  if (! summaryonly) {
    printf("--------------------------------------");
    printf("--------------------------------------\n");
  }
}
/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
  int readfifo = 0;
  int summaryonly = 0;
  int mmapmode = 0;
  int rxid = 0;
  ttrx_t *ttrx = 0;

  while (argc > 1) {
    if (strncmp(argv[1], "-f", 2) == 0) {
      readfifo = ! readfifo;
    } else if (strncmp(argv[1], "-s", 2) == 0) {
      summaryonly = ! summaryonly;
    } else if (strncmp(argv[1], "-m", 2) == 0) {
      mmapmode = ! mmapmode;
    } else if (argv[1][0] == '-' && isdigit(argv[1][1])) {
      rxid = argv[1][1] - '0';
    } else {
      printf("usage: dumprx [-<n>] [-f] [-m]\n");
      printf("options:\n");
      printf(" -<n>       select <n>-th TT-RX (<n>=0,1,2...9)\n");
      printf(" -f           read fifo\n");
      printf(" -s           summary only\n");
      printf(" -m           mmap mode\n");
      return 1;
    }
    argc--, argv++;
  }

  ttrx = open_ttrx(rxid, TTRX_RDONLY|(mmapmode ? TTRX_MMAPMODE : 0));
  if (! ttrx) {
    perror("open_ttrx");
    printf("TT-RX is not found.\n");
    exit(1);
  }

  regdump(ttrx, readfifo, summaryonly);
  return 0;
}
