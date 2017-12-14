/* ---------------------------------------------------------------------- *\
   jtagft.c

   For ft2u059, to receive IDCODE of FTSW2 Virtex-5

   Mikihiko Nakao, KEK IPNS

   2014080400  new, based on ft2u059.c-2014073000
   2015032700  only for ft2p -> ft2x
   2015040200  v5 status register revised, s6 implemented and tested
   2015040700  for cdc board
   2015040900  debug
   2015043000  xcf program debug
   2015080600  YM.Yook version
   2015082400  faster progmcsfile trial
   2015082500  fine tuning
   2015082501  renamed back to jtagft and clean up
   2015082502  serial mode as default
   2015082600  S3 trial (just started), chain cleanup
   2015082601  S6 bad status fix
   2015082606  S6 mcs trial - works!
   2015082607  S6 mcs cleanup
   2015082608  xcf16p/32p dependence
   2015082609  S3 trial - works!
   2015083100  restoring -f option
   2016021900  zynq idcode added from ise14.7
   2016030100  -f option for mcs
   2016040800  ft2u update
   2016040801  ft2u xport fix
   2016041100  mysleep(1000) restored
   2016041200  tck2p change
\* ---------------------------------------------------------------------- */

static char VERSION[] = "2016041200";

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "ftsw.h"
#include "xclist.h"

#define PROGRAM "jtagft"

typedef enum { XC3S, XC5V, XC6S, XC6V, XC7A, XC7K } fpgafamily_t;

/* #define LAST_IN_CHAIN 0  /* 1 for FTSW2-S3, FTSW1-V5, otherwise 0 */
int verbose  = 0;
int blocktdi = 1;
int ticktck  = 1;
int isft2p = 0;
int xjport = 0;
int dodump = 0;
int dofast = 0;
int donotwait = 0;
int doslow = 0;
int skipdr = 1;
int mysleepcount = 1000;
int usleep_update = 0;
int usleep_exit   = 0;
int ignoretdo = 0;

int xblkwrite = 0;
int xblkwait  = 0;
int xblksleep = 0; // or 500 for progbit
int xirwait = 1;

/* default for ftsw2 virtex5 */
int ndev_head = 0;
int nbit_head = 0;
int ndev_tail = 0;  /* 2 */
int nbit_tail = 0;  /* 16+6 */
int nbit_ir   = 10;
int nbit_dr   = 32;
int code_ir   = 0x3c9;
int xcfparallel = 0;

char debugtdo[1024];

#define FTSWREG_JTAGW  (0x1a0 >> 2)
#define FTSWREG_JTAGR  (0x1b0 >> 2)

#define D(a,b,c) (((a)>>(c))&((1<<((b)+1-(c)))-1))

/* ---------------------------------------------------------------------- *\
   findfamily
\* ---------------------------------------------------------------------- */
xfamily_t *
findfamily(char *name)
{
  int i;
  for (i=0; xfamlist[i].name; i++) {
    if (strcmp(name, xfamlist[i].name) == 0) return &xfamlist[i];
    if (strcmp(name, xfamlist[i].name+2) == 0) return &xfamlist[i];
  }
  return 0;
}
/* ---------------------------------------------------------------------- *\
   findfamilies
\* ---------------------------------------------------------------------- */
int
findfamilies(char *names, int *nbitp)
{
  char *name = malloc(strlen(names)+1);
  char *p, *q;
  xfamily_t *fam;
  int ndev = 0;
  strcpy(name, names);
  p = name;
  *nbitp = 0;
  while (p && *p) {
    q = strpbrk(p, "+-:,");
    if (q) *q++ = 0;
    if (! (fam = findfamily(p))) {
      printf("no such xilinx device %s\n", p);
      exit(1);
    }
    ndev++;
    *nbitp += fam->len;
    p = q;
  }
  free(name);
  return ndev;
}
/* ---------------------------------------------------------------------- *\
   mysleep
\* ---------------------------------------------------------------------- */
void
mysleep(int n)
{
  if (n > 0) mysleep(n - 1);
}
/* ---------------------------------------------------------------------- *\
   tdiblk2p
\* ---------------------------------------------------------------------- */
int
tdiblk2p(ftsw_t *ftsw, int ch)
{
  int ret;
  int i;
  write_ftsw(ftsw, FTSWREG_JTAGW, 0x2e0000 + (ch << 24) + xjport);

  xblkwrite++;
  for (i=0; i<10; i++) {
    ret = read_ftsw(ftsw, FTSWREG_JTAGR);
    if (((ret >> 29) & 1) == 0) break; // from ft3o033
    xblkwait++;
  }
  if (xblksleep) mysleep(xblksleep); // 500 for block

  return 0;
}
/* ---------------------------------------------------------------------- *\
   tdiblk
\* ---------------------------------------------------------------------- */
int
tdiblk(ftsw_t *ftsw, int ch)
{
  int ret;
  int i;

  if (isft2p) return tdiblk2p(ftsw, ch);

  write_ftsw(ftsw, FTSWREG_JTAGW, 0x0a00 + (ch << 24) + xjport);

  for (i=0; i<10; i++) {
    ret = read_ftsw(ftsw, FTSWREG_JTAGR);
    if ((ret & ~1) == 0) break;
  }
  if (xblksleep) mysleep(xblksleep); // 500 for block
  
  return 0;
}
/* ---------------------------------------------------------------------- *\
   tck2p
\* ---------------------------------------------------------------------- */
int
tck2p(ftsw_t *ftsw, int tms, int tdi, int show)
{
  int tck = 0x4e0000 + xjport;
  int bit = 0;
  int regval;

  regval = tck + (tdi?(1<<24):0) + (tms?(1<<23):0);
  write_ftsw(ftsw, FTSWREG_JTAGW, regval);

  bit = (read_ftsw(ftsw, FTSWREG_JTAGR) >> 31) & 1;

  if (dofast) {
    // it works with 500, marginal with 350, didn't work with 200
    //mysleep(500); // it works for ftsw!
    mysleep(1000); // trial for ecl collector
  } else {
    usleep(1);
  }

  if (dodump) {
    printf("%c%c%c%c %08x\n",
           '1', tms?'1':'0', tdi?'1':'0', bit?'1':'0', regval);
  } else if (show) {
    putchar('0' + bit);
  }

  return bit;
}
/* ---------------------------------------------------------------------- *\
   tck
\* ---------------------------------------------------------------------- */
int
tck(ftsw_t *ftsw, int tms, int tdi, int show)
{
  int bit;

  if (isft2p) return tck2p(ftsw, tms, tdi, show);
  
  if (ticktck) {
    write_ftsw(ftsw, FTSWREG_JTAGW,
               0x8200 + xjport + (tdi?0x1000:0) + (tms?0x2000:0));
  } else {
    write_ftsw(ftsw, FTSWREG_JTAGW,
               0x0200 + xjport + (tdi?0x1000:0) + (tms?0x2000:0));
    write_ftsw(ftsw, FTSWREG_JTAGW,
               0x4200 + xjport + (tdi?0x1000:0) + (tms?0x2000:0));
  }
  bit = read_ftsw(ftsw, FTSWREG_JTAGR) & 1;

  if (donotwait) {
    ;
  } else if (dofast) {
    // it works with 500, marginal with 350, didn't work with 200
    //mysleep(500); // it works for ftsw!
    mysleep(2000); // trial for ecl collector
  } else {
    usleep(1);
  }
  
  if (show) {
    putchar('0' + bit);
  }
  return bit;
}
/* ---------------------------------------------------------------------- *\
   reset
\* ---------------------------------------------------------------------- */
int
reset(ftsw_t *ftsw, int n)
{
  int i;

  if (verbose) printf("reset\n");
  
  for (i=0; i<n+2; i++) tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
  return 0;
}
/* ---------------------------------------------------------------------- *\
   runtest
\* ---------------------------------------------------------------------- */
int
runtest(ftsw_t *ftsw, int count)
{
  int i;

  if (verbose) printf("runtest %d\n", count);
  dofast = 1;
  for (i=0; i<4;     i++) tck(ftsw, 1, 0, 0);
  if (count < 10000) {
    for (i=0; i<count; i++) tck(ftsw, 0, 0, 0);
  } else {
    for (i=0; i<count; i++) {
      tck(ftsw, 0, 0, 0);
      if (i % (count/100) == 0) {
        printf("\r%d%%", i / (count/100));
        fflush(stdout);
      }
    }
    printf("\rerased.\n");
    fflush(stdout);
  }
  //for (i=0; i<3;     i++) tck(ftsw, 1, 0, 0);
  dofast = 0;
  return 0;
}
/* ---------------------------------------------------------------------- *\
   shift_tdi
\* ---------------------------------------------------------------------- */
unsigned int
shift_tdi(ftsw_t *ftsw, int nbit, int data, int show, int last)
{
  int i;
  int out = 0;
  char fmt[16];

  for (i=0; i<nbit; i++) {
    int tdi = (data >> i) & 1;
    int tms = (i == (nbit-1) && last) ? 1 : 0;
    out |= (tck(ftsw, tms, tdi, show) << i);
  }
  if (show) {
    sprintf(fmt, " (0x%%0%dx)\n", (nbit+3) / 4);
    printf(fmt, out);
  }
  return out;
}
/* ---------------------------------------------------------------------- *\
   shift_tdirev
\* ---------------------------------------------------------------------- */
unsigned int
shift_tdirev(ftsw_t *ftsw, int nbit, int data, int show, int last)
{
  int i;
  int out = 0;
  char fmt[16];

  for (i=nbit-1; i>=0; i--) {
    int tdi = (data >> i) & 1;
    int tms = (i == 0 && last) ? 1 : 0;
    out |= (tck(ftsw, tms, tdi, show) << i);
  }
  if (show) {
    sprintf(fmt, " (0x%%0%dx)\n", (nbit+3) / 4);
    printf(fmt, out);
  }
  return out;
}
/* ---------------------------------------------------------------------- *\
   head/tail
\* ---------------------------------------------------------------------- */
void
skip_ir(ftsw_t *ftsw, int nbit, int istail)
{
  int i;
  for (i = nbit; i > 0; i -= 32) {
    if (i > 32) {
      shift_tdi(ftsw, 32, 0xffffffff, 0, 0);
    } else {
      shift_tdi(ftsw, i, (1<<i)-1, 0, istail ? 1 : 0);
    }
  }
}
void
skip_dr(ftsw_t *ftsw, int ndev, int istail)
{
  int i;
  for (i = 0; i < ndev; i++) {
    if (i < ndev - 1) {
      shift_tdi(ftsw, 1, skipdr, 0, 0);
    } else {
      shift_tdi(ftsw, 1, skipdr, 0, istail ? 1 : 0);
    }
  }
}
/* ---------------------------------------------------------------------- *\
   shift_ir / shift_dr
   can start from TLR or EXIT[12]-[ID]R
\* ---------------------------------------------------------------------- */
unsigned int
shift_ir(ftsw_t *ftsw, int nbit, int data, int show)
{
  int last_in_chain = ndev_tail ? 0 : 1;
  unsigned int out = 0;
  
  if (show && verbose) {
    const char *p = "??????";
    if (nbit == 6) {
      switch (data) {
      case 0x3f: p = "bypass"; break;
      case 0x09: p = "idcode"; break;
      case 0x04: p = "cfgout"; break;
      case 0x05: p = "cfg_in"; break;
      case 0x0c: p = "jstart"; break;
      case 0x0b: p = "jprogr"; break;
      }
      printf("%02x-%s\n", data, p);
    } else {
      switch (data) {
      case 0x3ff: p = "bypass"; break;
      case 0x3c9: p = "idcode"; break;
      case 0x3c4: p = "cfgout"; break;
      case 0x3c5: p = "cfg_in"; break;
      case 0x3cc: p = "jstart"; break;
      case 0x3cb: p = "jprogr"; break;
      }
      printf("%03x-%s\n", data, p);
    }
  } else {
    if (xirwait) usleep(1);
  }
  
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
  tck(ftsw, 0, 0, 0);
  skip_ir(ftsw, nbit_head, 0);
  out = shift_tdi(ftsw, nbit, data, show, last_in_chain);
  skip_ir(ftsw, nbit_tail, 1);
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
  return out;
}
int
shift_dr(ftsw_t *ftsw, int nbit, int data, int show)
{
  int last_in_chain = ndev_tail ? 0 : 1;
  int out;
  
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
  tck(ftsw, 0, 0, 0);
  skip_dr(ftsw, ndev_head, 0);
  out = shift_tdi(ftsw, nbit, data, show, last_in_chain);
  skip_dr(ftsw, ndev_tail, 1);
  if (usleep_exit)   usleep(usleep_exit);
  tck(ftsw, 1, 0, 0);
  if (usleep_update) usleep(usleep_update);
  tck(ftsw, 0, 0, 0);
  return out;
}
void
shift_drbytes(ftsw_t *ftsw, int nbyte, const unsigned char *bytes)
{
  int last_in_chain = ndev_tail ? 0 : 1;
  int i;
  
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
  tck(ftsw, 0, 0, 0);
  skip_dr(ftsw, ndev_head, 0);

  for (i = 0; i < nbyte-1; i++) {
    if (blocktdi) {
      tdiblk(ftsw, bytes[i]);
    } else {
      donotwait = 1;
      shift_tdirev(ftsw, 8, bytes[i], 0, 0);
      donotwait = 0;
    }
  }
  if (blocktdi) {
    tdiblk(ftsw, bytes[i]);
  } else {
    donotwait = 1;
    shift_tdirev(ftsw, 8, bytes[nbyte-1], 0, last_in_chain);
    donotwait = 0;
  }

  if (isft2p) {
    for (i=0; i<10; i++) {
      int ret = read_ftsw(ftsw, FTSWREG_JTAGR);
      if (((ret >> 28) & 1) == 0) break;
    }
  }

  skip_dr(ftsw, ndev_tail, 1);
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
}
void
shift_drvec(ftsw_t *ftsw, int nvec, int *vec)
{
  int last_in_chain = ndev_tail ? 0 : 1;
  int i;
  
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
  tck(ftsw, 0, 0, 0);
  skip_dr(ftsw, ndev_head, 0);

  for (i = 0; i < nvec-1; i++) {
    shift_tdirev(ftsw, 32, vec[i], 0, 0);
  }
  shift_tdirev(ftsw, 32, vec[nvec-1], 0, last_in_chain);
  skip_dr(ftsw, ndev_tail, 1);
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
}
void
shift_drfile(ftsw_t *ftsw, int nbit, const char *filename)
{
  int last_in_chain = ndev_tail ? 0 : 1;
  int nbyte = (nbit + 3) / 4;
  int i;
  int j;
  char *buf = malloc(nbit);
  FILE *fp;
  if (! buf) { perror("malloc"); exit(1); }
  if (! (fp = fopen(filename, "r"))) { perror("fopen"); exit(1); }
  for (i=0, j=0; i<nbyte; ) {
    int c = fgetc(fp);
    if (c == EOF) { printf("EOF?\n"); exit(1); }
    if (c >= '0' && c <= '9') {
      buf[i++] = c - '0';
    } else if (c >= 'A' && c <= 'F') {
      buf[i++] = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
      buf[i++] = c - 'a' + 10;
    }
    j++;
  }
  fclose(fp);
  if (verbose) printf("%d / %d bytes\n", j, nbyte);
  
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
  tck(ftsw, 0, 0, 0);
  skip_dr(ftsw, ndev_head, 0);

  for (i = nbyte - 1; i > 0; i--) {
    shift_tdi(ftsw, 4, buf[i] & 15, 0, 0);
  }
  shift_tdi(ftsw, nbit - (nbyte-1)*4, buf[0] & 15, 0, last_in_chain);
  skip_dr(ftsw, ndev_tail, 1);
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);

  free(buf);
}
void
shift_drbitfile(ftsw_t *ftsw, FILE *fp, int size)
{
  int last_in_chain = ndev_tail ? 0 : 1;
  int i;
  int j;
  int n;
  int ch;
  int rev;

  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
  tck(ftsw, 0, 0, 0);
  skip_dr(ftsw, ndev_head, 0);

  printf("size = %d %d\n", size, size/20);
  n = 0;
  dofast = 1;
  ch = getc(fp); /* remant of old code, it was once ungetc'ed */
  do {
    int sav = ch;
    rev = ((ch >> 7) & 0x01)
      |   ((ch >> 5) & 0x02)
      |   ((ch >> 3) & 0x04)
      |   ((ch >> 1) & 0x08)
      |   ((ch << 1) & 0x10)
      |   ((ch << 3) & 0x20)
      |   ((ch << 5) & 0x40)
      |   ((ch << 7) & 0x80);
    ch = getc(fp);
    if (ch == EOF) {
      shift_tdirev(ftsw, 8, sav, 0, last_in_chain);
    } else if (blocktdi) {
      tdiblk(ftsw, sav);
    } else {
      donotwait = 1;
      shift_tdirev(ftsw, 8, sav, 0, 0);
      donotwait = 0;
    }

    if ((n % (size/100)) == 0) {
      printf("\r%d%%", n / (size/100) * 1);
      fflush(stdout);
    }
    n++;
    
  } while (ch != EOF);
  dofast = 0;
  usleep(100000);

  printf("\r");

  skip_dr(ftsw, ndev_tail, 1);
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);

  fclose(fp);
}
/* ---------------------------------------------------------------------- *\
   famcode
\* ---------------------------------------------------------------------- */
fpgafamily_t
famcode(char *device)
{
  if (strncmp(device, "3s", 2) == 0) {
    return XC3S;
  } else if (strncmp(device, "5v", 2) == 0) {
    return XC5V;
  } else if (strncmp(device, "6v", 2) == 0) {
    return XC6V;
  } else if (strncmp(device, "6s", 2) == 0) {
    return XC6S;
  } else if (strncmp(device, "7a", 2) == 0) {
    return XC7A;
  } else if (strncmp(device, "7k", 2) == 0) {
    return XC7K;
  } else {
    printf("yet unsupported device %s\n", device);
    exit(1);
  }
}
/* ---------------------------------------------------------------------- *\
   openmcsfile
\* ---------------------------------------------------------------------- */
int
gethex(const char *ptr, int len)
{
  int val = 0;
  while (len-- > 0) {
    int c = tolower(*ptr++);
    val *= 16;
    if (isdigit(c)) {
      val += c - '0';
    } else if (c >= 'a' && c <= 'f') {
      val += c - 'a' + 10;
    } else {
      return -1;
    }
  }
  return val;
}

const unsigned char *
openmcsfile(const char *filename, int *mcssizep)
{
  FILE *fp;
  char buf[256];
  char *mcsbuf;
  char *p;
  struct stat statbuf;
  int i = 0;
  int j = 0;
  int linesz = 0;
  int iscrlf = 0;
  int size = 0;
  int remnant = 0;
  
  if (stat(filename, &statbuf) < 0) { perror("stat"); exit(1); }
  if (! (fp = fopen(filename, "r"))) { perror("fopen"); exit(1); }
  if (! (fgets(buf, sizeof(buf), fp))) { perror("fgets"); exit(1); }

  if (*buf && buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = 0;
  if (*buf && buf[strlen(buf)-1] == '\r') {
    buf[strlen(buf)-1] = 0;
    iscrlf = 1;
  }
  
  if (strcmp(buf, ":020000040000FA") != 0) {
    printf("unknown mcs first line %s\n", buf);
  }

  /* (44*4096 + 16) * n + 12 */
  /* 20150825: there is a case a CR is attached only at the last line */
  linesz = 1+8+32+2+iscrlf+1;
  remnant = (statbuf.st_size - 12) % (linesz*4096+(16+iscrlf));
  if (remnant == 0 || remnant == 1) {
    size = ((statbuf.st_size - 12) / (linesz*4096+(16+iscrlf))) * (16*4096);
  } else {
    printf("size error: %d byte is not (multiple of %d*4096+%d) + 12 or 13\n",
           statbuf.st_size, linesz, 16+iscrlf);
    exit(1);
  }

  if (! (fgets(buf, sizeof(buf), fp))) { perror("fgets"); exit(1); }
  
  if (! (mcsbuf = malloc(statbuf.st_size/2))) { perror("malloc"); exit(1); }

  p = mcsbuf;

  for (i=0; ; i++) {
    int xorsum = 0;
    char tmpbuf[256];
    int a;

    if (strncmp(buf, ":00000001", 9) == 0) {
      break;
    } else if (strncmp(buf, ":02000004", 9) == 0) {
      i--;
      if (! (fgets(buf, sizeof(buf), fp))) { perror("fgets"); exit(1); }
      continue;
    }
    
    if (*buf && buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = 0;
    if (*buf && buf[strlen(buf)-1] == '\r') buf[strlen(buf)-1] = 0;

    linesz = (strlen(buf)-11) / 2;

    if (buf[0] != ':') goto bad_format;
    if (gethex(buf+1, 2) != linesz) goto bad_format;
    if (gethex(buf+3, 4) != ((i*linesz) & 0xffff)) goto bad_format;
    if (gethex(buf+7, 2) != 0) goto bad_format;

    for (j=0; j<linesz+5; j++) xorsum += gethex(buf+1+j*2, 2);
    if (xorsum & 0xff) {
      printf("xorsum=%02x\n", xorsum & 0xff);
      goto bad_format;
    }

    for (j=0; j<linesz; j++) *p++ = gethex(buf+9+j*2, 2);
    
    if (! (fgets(buf, sizeof(buf), fp))) { perror("fgets"); exit(1); }
  }
  
  if (gethex(buf+9, 2) != 0xff) goto bad_format;

  if (mcssizep) *mcssizep = size;
  fclose(fp);
  return mcsbuf;

 bad_format:
  fclose(fp);
  printf("unknown MCS format\n");
  exit(1);
}
/* ---------------------------------------------------------------------- *\
   erasexcf
\* ---------------------------------------------------------------------- */
int
erasexcf(ftsw_t *ftsw)
{
  int i;
  int NBIT_IR = 16;
  int IR_IDCODE         = 0x00fe;
  int IR_ISPEN          = 0x00e8;
  int IR_XSC_DATA_RDPT  = 0x0004;
  int IR_XSC_DATA_WRPT  = 0x00f7;
  int IR_ISC_ERASE      = 0x00ec;
  int IR_XSC_OP_STATUS  = 0x00e3;
  int IR_CONLD          = 0x00f0;
  int IR_XSC_UNLOCK     = 0xaa55;
  int IR_BYPASS         = 0xffff;

  printf("erasing xcf...\n");
  
  /* ispen */
  shift_ir(ftsw, NBIT_IR, IR_ISPEN, verbose);
  shift_dr(ftsw, 8, 0x03, verbose);

  /* xsc_data_rdpt */
  shift_ir(ftsw, NBIT_IR, IR_XSC_DATA_RDPT, verbose);
  shift_dr(ftsw, 16, 0, verbose);

  /* xsc_data_wrpt */
  shift_ir(ftsw, NBIT_IR, IR_XSC_DATA_WRPT, verbose);
  shift_dr(ftsw, 16, 0, verbose);

  /* ispen */
  skipdr =0;
  shift_ir(ftsw, NBIT_IR, IR_ISPEN, verbose);
  shift_dr(ftsw, 8, 0xd0, verbose);

  /* xsc_unlock */
  shift_ir(ftsw, NBIT_IR, IR_XSC_UNLOCK, verbose);
  shift_dr(ftsw, 24, 0x0003f, verbose);

  /* isc_erase */
  shift_ir(ftsw, NBIT_IR, IR_ISC_ERASE, verbose);
  shift_dr(ftsw, 24, 0x0003f, verbose);

  /* loop xsc_op_status */
  shift_ir(ftsw, NBIT_IR, IR_XSC_OP_STATUS, verbose);
  i = 0;
  while (1) {
    static int prev = 0;
    int ret;
    ret = shift_dr(ftsw, 8, 0, verbose);
    if (ret != prev) printf("xsc_op_status: ret=%04x\n", ret);
    prev = ret;
    /*
    if ((ret & ~0x47) != 0x30) {
      printf("xsc_op_status: unknown ret=%04x\n", ret);
      exit(1);
    }
    */
    i++;
    if (ret & 4) break;
    usleep(10000);
  }
  printf("erase done (wait=%d)\n", i);

  return 0;
}
/* ---------------------------------------------------------------------- *\
   progmcsfile
\* ---------------------------------------------------------------------- */
int
progmcsfile(ftsw_t *ftsw, char *filename)
{
  int i;
  int NBIT_IR = 16;
  int IR_IDCODE         = 0x00fe;
  int IR_ISPEN          = 0x00e8;
  int IR_XSC_DATA_RDPT  = 0x0004;
  int IR_XSC_DATA_WRPT  = 0x00f7;
  int IR_ISC_PROGRAM    = 0x00ea;
  int IR_ISC_ADDR_SHIFT = 0x00eb;
  int IR_ISC_ERASE      = 0x00ec;
  int IR_XSC_OP_STATUS  = 0x00e3;
  int IR_ISC_DATA_SHIFT = 0x00ed;
  int IR_CONLD          = 0x00f0;
  int IR_XSC_UNLOCK     = 0xaa55;
  int IR_XSC_DATA_SUCR  = 0x000e;
  int IR_XSC_DATA_CC    = 0x0007;
  int IR_XSC_DATA_CCB   = 0x000c;
  int IR_XSC_DATA_DONE  = 0x0009;
  int IR_XSC_DATA_BTC   = 0x00f2;
  int IR_BYPASS         = 0xffff;
  unsigned int idcode;
  int mcssize = 0;
  const unsigned char *mcsbytes = openmcsfile(filename, &mcssize);
  const unsigned char *p = mcsbytes;
  unsigned char revtbl[256];
  int prevret;
  enum { XCFNOT, XCF08P, XCF16P, XCF32P } xcftype;

  for (i=0; i<256; i++) {
    revtbl[i] = ((i&0x80)>>7) | ((i&0x40)>>5) | ((i&0x20)>>3) | ((i&0x10)>>1)
      |         ((i&0x01)<<7) | ((i&0x02)<<5) | ((i&0x04)<<3) | ((i&0x08)<<1);
  }

  printf("size=%d ", mcssize);
  
  if (xcfparallel) {
    printf("parallel mode (-P option is specified)\n");
  } else {
    printf("serial mode (use -P option for parallel mode)\n");
  }
  
  /* check idcode */
  reset(ftsw, 3);

  shift_ir(ftsw, NBIT_IR, IR_IDCODE, verbose);
  idcode = shift_dr(ftsw, 32, 0x00000000, verbose);
  
  switch (idcode & 0x0fffffff) {
  case 0x05058093: xcftype = XCF16P; break;
  case 0x05059093: xcftype = XCF32P; break;
  case 0x05057093: xcftype = XCF08P; break;
  default: xcftype = XCFNOT;
  }

  for (i=0; xidlist[i].id; i++) {
    if ((idcode & xidlist[i].mask) == xidlist[i].id) break;
  }
  
  if (! xidlist[i].id) {
    printf("idcode = %08x is unknown\n", idcode);
    if (! ignoretdo) return -1;
  } else if (strncmp(xidlist[i].name, "xcf", 3) != 0 ||
             xidlist[i].name[5] != 'p') {
    printf("idcode = %08x %s is not supported\n", idcode, xidlist[i].name);
    if (! ignoretdo) return -1;
  }

  if (erasexcf(ftsw)) return -1;

  skipdr = 0;
  
  /* conld */
  shift_ir(ftsw, NBIT_IR, IR_CONLD,  verbose);

  /* ispen */
  shift_ir(ftsw, NBIT_IR, IR_ISPEN, verbose);
  shift_dr(ftsw, 8, 0xd0, verbose);

  reset(ftsw, 3);
  skipdr = 1;
  
  /* ispen */
  shift_ir(ftsw, NBIT_IR, IR_ISPEN, verbose);
  shift_dr(ftsw, 8, 0x03, verbose);
  usleep(200);
  shift_ir(ftsw, NBIT_IR, IR_ISPEN, verbose);
  shift_dr(ftsw, 8, 0x03, verbose);
  usleep(200);
  shift_ir(ftsw, NBIT_IR, IR_XSC_DATA_BTC, verbose);
  /* see ug161 v1.5 (xcf) p.81 */
  switch (xcftype) {
  case XCF08P: shift_dr(ftsw, 32, 0xffffffe0, verbose); break; /* guess */
  case XCF16P: shift_dr(ftsw, 32, 0xffffffe4, verbose); break; /* ftsw2 */
  case XCF32P: shift_dr(ftsw, 32, 0xffffffec, verbose); break; /* col */
  }
  
  shift_ir(ftsw, NBIT_IR, IR_ISC_PROGRAM, verbose);
  shift_ir(ftsw, NBIT_IR, IR_XSC_OP_STATUS, verbose);
  usleep(100);
  shift_dr(ftsw, 8, 0x00, verbose);
  shift_dr(ftsw, 8, 0x00, verbose);
  
  skipdr = 0;
  shift_ir(ftsw, NBIT_IR, IR_ISPEN, verbose);
  /* shift_dr(ftsw, 8, 0x03, verbose); /* ftsw2? */
  shift_dr(ftsw, 8, 0xd0, verbose); /* col */
  
  /* printf("before loop %d\n", mcssize/32); */
  usleep_exit = 0; // 20150824 trial
  usleep_update = 0; // 20150824 trial
  xblkwrite = 0; // 20150824 trial
  xblkwait  = 0; // 20150824 trial
  xirwait = 0; // 20150824 trial
  dofast = 1; // 20150824 trial
  prevret = 0;
  
  for (i=0; i<mcssize; i+=32) {
    int ret;
    unsigned char revbuf[32];
    int j;

    //printf(".");  fflush(stdout);
    for (j=0; j<32; j++) {
      revbuf[j] = revtbl[p[j]];
    }
    
    /* isc_data_shift */
    shift_ir(ftsw, NBIT_IR, IR_ISC_DATA_SHIFT, verbose);
    shift_drbytes(ftsw, 32, revbuf);

    //printf(".");  fflush(stdout);
    p += 32;

#if 0
    if (i == 0) {
      /* isc_addr_shift (worked for FTSW xcf16p) */
      shift_ir(ftsw, NBIT_IR, IR_ISC_ADDR_SHIFT, verbose);
      shift_dr(ftsw, 24, 0, verbose);
    }
#else
    if ((i % 0x100000) == 0) {
      /* isc_addr_shift (for ECL xcf32p, need to check with FTSW) */
      shift_ir(ftsw, NBIT_IR, IR_ISC_ADDR_SHIFT, verbose);
      shift_dr(ftsw, 24, i, verbose);
    }
#endif

    /* isc_program */
    shift_ir(ftsw, NBIT_IR, IR_ISC_PROGRAM, verbose);

    /* xsc_op_status */
    prevret = 0;
    do {
      shift_ir(ftsw, NBIT_IR, IR_XSC_OP_STATUS, verbose);
      ret = shift_dr(ftsw, 8, 0, verbose);
      prevret++;
    } while (ret != 0x36 && prevret <= 10);
    if (prevret == 10) printf("\nopstatus = %02x (%d)\n", ret, prevret);
    if (((i/32) % ((mcssize/32)/100)) == 0) {
      printf("\r%d%% %d %d", (i/32) / ((mcssize/32)/100),
             xblkwrite, xblkwait);
      fflush(stdout);
    }
  }
  xirwait = 1;
  free((char *)mcsbytes);
  p = mcsbytes = 0;

  /* ispen */
  reset(ftsw, 3);
  skipdr = 1;
  for (i=0; i<3; i++) {
    shift_ir(ftsw, NBIT_IR, IR_ISPEN, verbose);
    shift_dr(ftsw, 8, 0x03, verbose);
    usleep(240); // trial
  }

  /* xsc_data_sucr */
  usleep_exit = 240; // trial
  shift_ir(ftsw, NBIT_IR, IR_XSC_DATA_SUCR, verbose);
  shift_dr(ftsw, 16, 0, verbose);
  usleep_exit = 0; // trial

  /* ispen */
  shift_ir(ftsw, NBIT_IR, IR_ISPEN, verbose);
  shift_dr(ftsw, 8, 0x03, verbose);
  usleep(240); // trial
    
  /* xsc_unlock */
  shift_ir(ftsw, NBIT_IR, IR_XSC_UNLOCK, verbose);
  shift_dr(ftsw, 24, 0x000020, verbose);

#if 1
  /* isc_erase (special) */
  /* probably it meant this, but impact does "else" */
#if 1
  shift_ir(ftsw, NBIT_IR, IR_ISC_ERASE, verbose);
  shift_dr(ftsw, 24, 0x20, verbose);
#else
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
  tck(ftsw, 0, 0, 0);
  skip_ir(ftsw, nbit_head, 0);
  shift_tdi(ftsw, NBIT_IR, IR_ISC_ERASE, 0, 0 /*last_in_chain*/);
  skip_ir(ftsw, nbit_tail, 1);
  tck(ftsw, 1, 0, 0); /* ir_update */
  tck(ftsw, 1, 0, 0); /* ir_select */
  tck(ftsw, 0, 0, 0); /* ir_capture */
  tck(ftsw, 0, 0, 0); /* ir_shift */
  /* 1100000000-0000000000100000 */
  /* 11 0000 0000-0000 0000 0010 0000 */
  shift_tdi(ftsw, 26, 0x3000020, 0, 1);
  tck(ftsw, 1, 0, 0); /* ir_update */
  tck(ftsw, 0, 0, 0); /* rt_idle */
#endif
#else
  /* suspicious code: and it works only for ftsw */
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
  tck(ftsw, 0, 0, 0);
  skip_ir(ftsw, nbit_head, 0);
  shift_tdi(ftsw, NBIT_IR, IR_ISC_ERASE, 0, 0 /*last_in_chain*/);
  skip_ir(ftsw, nbit_tail, 1);
  tck(ftsw, 1, 0, 0); /* ir_update */
  tck(ftsw, 1, 0, 0); /* ir_select */
  tck(ftsw, 0, 0, 0); /* ir_capture */
  shift_tdi(ftsw, 26, (1<<25)|(0x20<<1)|(1<<0), 0, 1);
  tck(ftsw, 1, 0, 0); /* ir_update */
  tck(ftsw, 0, 0, 0); /* rt_idle */
#endif

  /* xsc_op_status */
  shift_ir(ftsw, NBIT_IR, IR_XSC_OP_STATUS, verbose);
  /* printf("t=%ld\n", time(0)); */
  i=0;
  while (1) {
    static int prev=0;
    int ret;
    usleep(2500);
    usleep_exit = 200;
    ret = shift_dr(ftsw, 8, 0, verbose);
    usleep_exit = 0;
    if (ret != prev) printf("xsc_op_status: ret=%04x\n", ret);
    prev = ret;
    if (ret & 4) break;
    i++;
  }
  usleep_exit = 400;
  shift_dr(ftsw, 8, 0, verbose);
  usleep_exit = 0;
  /* printf("t=%ld count=%d\n", time(0), i); */

  /* xsc_data_sucr */
  shift_ir(ftsw, NBIT_IR, IR_XSC_DATA_SUCR, verbose);
  shift_dr(ftsw, 16, 0xfffc, verbose);
  
  /* isc_program */
  shift_ir(ftsw, NBIT_IR, IR_ISC_PROGRAM, verbose);

  /* op_status */
  shift_ir(ftsw, NBIT_IR, IR_XSC_OP_STATUS, verbose);
  usleep_exit = 200;
  shift_dr(ftsw, 8, 0, verbose);
  shift_dr(ftsw, 8, 0, verbose);
  usleep_exit = 0;

  /* xsc_data_sucr */
  shift_ir(ftsw, NBIT_IR, IR_XSC_DATA_SUCR, verbose);
  shift_dr(ftsw, 16, 0x0000, verbose);

  /* ispen */
  shift_ir(ftsw, NBIT_IR, IR_ISPEN, verbose);
  shift_dr(ftsw, 8, 0x03, verbose);
  usleep(1000); // trial

  /* xsc_data_ccb */
  shift_ir(ftsw, NBIT_IR, IR_XSC_DATA_CCB, verbose);
  if (xcfparallel) {
    printf("setting xcf parallel mode.\n");
    shift_dr(ftsw, 16, 0xfff9, verbose); /* parallel */
  } else {
    printf("setting xcf serial mode.\n");
    shift_dr(ftsw, 16, 0xffff, verbose); /* serial */
  }
  
  /* isc_program */
  shift_ir(ftsw, NBIT_IR, IR_ISC_PROGRAM, verbose);

  /* op_status */
  shift_ir(ftsw, NBIT_IR, IR_XSC_OP_STATUS, verbose);
  usleep(100);
  usleep_exit = 200;
  shift_dr(ftsw, 8, 0, verbose);
  shift_dr(ftsw, 8, 0, verbose);
  usleep_exit = 0;

  /* conld */
  shift_ir(ftsw, NBIT_IR, IR_CONLD, verbose);
  usleep(200);
  reset(ftsw, 3);

  /* ispen */
  shift_ir(ftsw, NBIT_IR, IR_ISPEN, verbose);
  shift_dr(ftsw, 8, 0x03, verbose);

  /* xsc_data_done */
  usleep(1000); // trial
  //usleep(200);
  shift_ir(ftsw, NBIT_IR, IR_XSC_DATA_DONE, verbose);
  usleep_exit = 300;
  shift_dr(ftsw, 8, 0x00, verbose);
  usleep_exit = 0;
  shift_ir(ftsw, NBIT_IR, IR_XSC_DATA_DONE, verbose);
#if 1 /* collector */
  shift_dr(ftsw, 8, 0xc0, verbose);
#else /* ftsw */
  shift_dr(ftsw, 8, 0xcc, verbose);
#endif

  /* isc_program */
  shift_ir(ftsw, NBIT_IR, IR_ISC_PROGRAM, verbose);

  /* op_status */
  shift_ir(ftsw, NBIT_IR, IR_XSC_OP_STATUS, verbose);
  usleep(100);
  //usleep_exit = 300;
  usleep_exit = 700; //trial
  shift_dr(ftsw, 8, 0, verbose);
  shift_dr(ftsw, 8, 0, verbose);
  usleep_exit = 0;

  reset(ftsw, 8);
  
  /* conld */
  shift_ir(ftsw, NBIT_IR, IR_CONLD, verbose);
  reset(ftsw, 3);

  /* bypass */
  shift_ir(ftsw, NBIT_IR, IR_BYPASS, verbose);
  skipdr = 0;
  shift_dr(ftsw, 1, 0, verbose);

  printf("\rdone.\n");
}
/* ---------------------------------------------------------------------- *\
   openbitfile
\* ---------------------------------------------------------------------- */
FILE *
openbitfile(const char *filename, int *sizep, fpgafamily_t *famp)
{
  FILE *fp;
  int size;
  int i;
  int ch;
  struct stat statbuf;
  char device[256];

  if (stat(filename, &statbuf) < 0) { perror("stat"); exit(1); }
  size = statbuf.st_size;
  
  if (! (fp = fopen(filename, "r"))) { perror("fopen"); exit(1); }
  
  for (i = 0; i<16; i++) ch = getc(fp); /* skip first 16 bytes */

  while ((ch = getc(fp)) != 0x00 && ch != EOF) {
    ;
  }
  ch = getc(fp);
  ch = getc(fp);
  ch = getc(fp);
  for (i=0; (ch = getc(fp)) != 0x00 && ch != EOF && i<256; i++) {
    device[i] = ch;
  }
  device[i] = 0;
  printf("device=%s\n", device);
  *famp = famcode(device);

  while ((ch = getc(fp)) != 0xff && ch != EOF) {
    ;
  }
  if (ch == EOF) {
    fprintf(stderr, "immature EOF for %s\n", filename);
    exit(1);
  }

  *sizep = size;
  return fp;
}
/* ---------------------------------------------------------------------- *\
   progbitfile
\* ---------------------------------------------------------------------- */
int
progbitfile(ftsw_t *ftsw, char *filename)
{
  int i;
  int ret;
  int idcode;
  int status;

  /*
     Virtex-5 UG191v3.5 p.114-
     type 1 packet header format
       header-type [31:29] = 001
       opcode      [28:27] = 00:NOP 01:Read 10:Write 11:Reserved
       rsv         [26:18] = 000000000
       reg.-addr.  [17:13] = 7:STAT
       rsv         [12:11] = 00
       word count  [10:0]

     status register 0x3f3e0bc0 (now)
     0000 0011 1101 0000 0111 1100 1111 1100 (reverse bit order)
     status register 0x3ffe0b80 (previous expected)
     0000 0001 1101 0000 0111 1111 1111 1100
     mask 0xff1ffc1f => expect 0x3f1e0b80
     1111 1001 1111 1111 1111 1000 1111 1111
     (don't care bit [26:25] = bus_width / bit [10:8]  = M[2:0])

     (Virtex-6 UG380v3.8 p.118)
     status register looks similar but with more bits are used.
     

     Spartan-6 UG380v2.2 p.89-
     type 1 packet header format
       header-ytpe [15:13] = 001
       operation   [12:11] = 00:NOP 01:Read 10:Write
       reg.-addr.  [10:5]  = 8:STAT
       word count  [4:0]   = 1
       001 01 00100 00001 = 0010 1001 0000 0001 = 2901

     status register 0x1cec (now)
     stat 0011 0111 0011 1000 (DONE:13 INIT_B:12 M012:11-9)
     mask 1111 0000 1111 1111 = 0xff0f (DCMLOCK:2)
     exp  0011 0000 0011 1000 = 0x1c0c
  */

  int *pkt_readstat = 0;
  int nword_readstat = 0;
  int statmask = -1;
  int statexp  = 0;
  
  /* 001 01 000000000 00111 00 00000000001 */
  int v5statmask = 0xff1ffc1f;
  int v5statexp  = 0x3f1e0800;
  /* 3cec from collector:
     15: syncwordwatchdog_strikeout (error)
     14: in_pwrdn
     13: done
     12: init_b
     11:9 mode
     8: hswap_en
     3: gts_cfg_b
     2: dcm_lock
     1: id_error
     0: crc_error
   */
  int s6statmask = 0x0000d00f;
  int s6statexp  = 0x0000100c;
  int s3statmask = 0x0000d00f;
  int s3statexp  = 0x0000100c;

  static int v5readstat[5] = {
    0xffffffff,
    0xaa995566,
    0x20000000,  /* 001 00 ... */
    0x2800e001,  /* Read STAT(7) n=1 */
    0x00000000 };
  
  static int s6readstat[4] = {
    0xffffffff,
    0xaa995566,
    0x20002901,
    0x00000000 };
    
  /* must be short */
  static int s3readstat[] = {
    0xffffffff,
    0xffffaa99,
    0x29012000,
    0x20000000 };
    
  const int NBIT_DR = 32;

  /* VIR for all V5 and V6 */
  const int NBIT_VIR = 10;
  const int VIR_IDCODE = 0x3c9;
  const int VIR_JPROG  = 0x3cb;
  const int VIR_JSTART = 0x3cc;
  const int VIR_CFGIN  = 0x3c5;
  const int VIR_CFGOUT = 0x3c4;
  
  /* SIR for all S3, S6, A7, K7 and most of V7 */
  const int NBIT_SIR = 6;
  const int SIR_IDCODE = 0x09;
  const int SIR_JPROG  = 0x0b;
  const int SIR_JSTART = 0x0c;
  const int SIR_CFGIN  = 0x05;
  const int SIR_CFGOUT = 0x04;

  int NBIT_IR;
  int IR_IDCODE;
  int IR_JPROG;
  int IR_JSTART;
  int IR_CFGIN;
  int IR_CFGOUT;

  int filesize = 0;
  fpgafamily_t family;
  int vir = 0;

  FILE *fp = openbitfile(filename, &filesize, &family);

  if (family == XC5V || family == XC6V) {
    vir = 1;
    statmask = v5statmask;
    statexp  = v5statexp;
    pkt_readstat = v5readstat;
    nword_readstat = sizeof(v5readstat);
    NBIT_IR   = NBIT_VIR;
    IR_IDCODE = VIR_IDCODE;
    IR_JPROG  = VIR_JPROG;
    IR_JSTART = VIR_JSTART;
    IR_CFGIN  = VIR_CFGIN;
    IR_CFGOUT = VIR_CFGOUT;
  } else if (family == XC3S) {
    vir = 0;
    statmask = s3statmask;
    statexp  = s3statexp;
    pkt_readstat = s3readstat;
    nword_readstat = sizeof(s3readstat);
    NBIT_IR   = NBIT_SIR;
    IR_IDCODE = SIR_IDCODE;
    IR_JPROG  = SIR_JPROG;
    IR_JSTART = SIR_JSTART;
    IR_CFGIN  = SIR_CFGIN;
    IR_CFGOUT = SIR_CFGOUT;
  } else {
    vir = 0;
    statmask = s6statmask;
    statexp  = s6statexp;
    pkt_readstat = s6readstat;
    nword_readstat = sizeof(s6readstat);
    NBIT_IR   = NBIT_SIR;
    IR_IDCODE = SIR_IDCODE;
    IR_JPROG  = SIR_JPROG;
    IR_JSTART = SIR_JSTART;
    IR_CFGIN  = SIR_CFGIN;
    IR_CFGOUT = SIR_CFGOUT;
  }
  
  reset(ftsw, 3);

  shift_ir(ftsw, NBIT_IR, IR_IDCODE, verbose);
  idcode = shift_dr(ftsw, NBIT_DR, 0x00000000, verbose);
  printf("idcode = %08x\n", idcode);

  shift_ir(ftsw, NBIT_IR, IR_JPROG, verbose);

  for (i=0; i<10; i++) {
    ret = shift_ir(ftsw, NBIT_IR, IR_CFGIN, 0); /* cfg_in */

    if (vir && (ret & ~0x10) != 0x3c1) { /* is it V5 only? */
      if (verbose) {
	printf("something is wrong out=%x i=%d\n", ret, i);
      }
      if (! ignoretdo) return -1;
    }
    if (! vir && (ret & ~0x10) != 1) { /* is it S6 only? */
      if (verbose) {
	printf("something is wrong out=%x i=%d\n", ret, i);
      }
      if (! ignoretdo) return -1;
    }
    if ((ret & 0x10)) break; /* is it V5 only? */
    if (ignoretdo) break;
  }
  if (i == 10) {
    if (verbose) printf("init did not go up out=%x\n", ret);
    return -1;
  }
  if (verbose) printf("%d times waited\n", i);

  shift_ir(ftsw, NBIT_IR, IR_CFGIN, verbose); /* cfg_in */
  
  if (verbose) printf("file %s\n", filename);

  shift_drbitfile(ftsw, fp, filesize);

  shift_ir(ftsw, NBIT_IR, IR_JSTART, verbose); /* jstart */

  runtest(ftsw, 13);

  doslow = 1;
  
  shift_ir(ftsw, NBIT_IR, IR_CFGIN, verbose); /* cfg_in */

  if (verbose) printf("readstat\n");
  shift_drvec(ftsw, nword_readstat, pkt_readstat);
  
  status = shift_ir(ftsw, NBIT_IR, IR_CFGOUT, verbose); /* cfg_out */
  printf("status = %03x (%s)", status, status & 0x20 ? "done" : "not-done");
  if (verbose) printf("\n");
  status = shift_dr(ftsw, NBIT_DR, 0, verbose);
  if (! verbose) printf(" %08x\n", status);
  if ((status & statmask) != statexp) {
    printf("bad status\n");
    if (! ignoretdo) return -1;
  }
  ret = read_ftsw(ftsw, FTSWREG_JTAGR);
  status = isft2p ? D(ret,28,24) : D(ret,7,3);
  if (status) {
    printf("jtag error %x (%08x)\n", status, ret);
    if (! ignoretdo) return -1;
  }

  printf("done.\n");
  return 0;
}
/* ---------------------------------------------------------------------- *\
   progft
\* ---------------------------------------------------------------------- */
int
progft(ftsw_t *ftsw, char *filename)
{
  int len = strlen(filename);
  if (len > 4 && strcmp(filename + len - 4, ".bit") == 0) {
    progbitfile(ftsw, filename);
  } else if (len > 4 && strcmp(filename + len - 4, ".mcs") == 0) {
    printf("progmcsfile %s\n", filename);
    progmcsfile(ftsw, filename);
  } else {
    printf("jtagft: unknown file type for %s\n", filename);
  }
}
/* ---------------------------------------------------------------------- *\
   readstatft
\* ---------------------------------------------------------------------- */
int
readstatft(ftsw_t *ftsw, char *famname)
{
  int i;
  int ret;
  int idcode;
  int status;

  int *pkt_readstat = 0;
  int nword_readstat = 0;
  int statmask = -1;
  int statexp  = 0;
  
  static int v5readstat[5] = {
    0xffffffff,
    0xaa995566,
    0x20000000,  /* 001 00 ... */
    0x2800e001,  /* Read STAT(7) n=1 */
    0x00000000 };
  /* 001 01 000000000 00111 00 00000000001 */
  int v5statmask = 0xff1ffc1f;
  int v5statexp  = 0x3f1e0800;
  int s6statmask = 0xffffff0f;
  int s6statexp  = 0x00001c0c;

  static int s6readstat[4] = {
    0xffffffff,
    0xaa995566,
    0x20002901,
    0x00000000 };
    
  const int NBIT_DR = 32;

  /* VIR for all V5 and V6 */
  const int NBIT_VIR = 10;
  const int VIR_IDCODE = 0x3c9;
  const int VIR_JPROG  = 0x3cb;
  const int VIR_JSTART = 0x3cc;
  const int VIR_CFGIN  = 0x3c5;
  const int VIR_CFGOUT = 0x3c4;
  
  /* SIR for all S3, S6, A7, K7 and most of V7 */
  const int NBIT_SIR = 6;
  const int SIR_IDCODE = 0x09;
  const int SIR_JPROG  = 0x0b;
  const int SIR_JSTART = 0x0c;
  const int SIR_CFGIN  = 0x05;
  const int SIR_CFGOUT = 0x04;

  int NBIT_IR;
  int IR_IDCODE;
  int IR_JPROG;
  int IR_JSTART;
  int IR_CFGIN;
  int IR_CFGOUT;

  int filesize = 0;
  fpgafamily_t family;
  int vir = 0;

  family = famcode(famname);

  if (family == XC5V || family == XC6V) {
    vir = 1;
    statmask = v5statmask;
    statexp  = v5statexp;
    pkt_readstat = v5readstat;
    nword_readstat = sizeof(v5readstat);
    NBIT_IR   = NBIT_VIR;
    IR_IDCODE = VIR_IDCODE;
    IR_JPROG  = VIR_JPROG;
    IR_JSTART = VIR_JSTART;
    IR_CFGIN  = VIR_CFGIN;
    IR_CFGOUT = VIR_CFGOUT;
  } else {
    vir = 0;
    statmask = s6statmask;
    statexp  = s6statexp;
    pkt_readstat = s6readstat;
    nword_readstat = sizeof(s6readstat);
    NBIT_IR   = NBIT_SIR;
    IR_IDCODE = SIR_IDCODE;
    IR_JPROG  = SIR_JPROG;
    IR_JSTART = SIR_JSTART;
    IR_CFGIN  = SIR_CFGIN;
    IR_CFGOUT = SIR_CFGOUT;
  }
  
  reset(ftsw, 3);

  shift_ir(ftsw, NBIT_IR, IR_IDCODE, verbose);
  idcode = shift_dr(ftsw, NBIT_DR, 0x00000000, verbose);
  printf("idcode = %08x\n", idcode);

  shift_ir(ftsw, NBIT_IR, IR_CFGIN, verbose); /* cfg_in */

  if (verbose) printf("readstat\n");
  shift_drvec(ftsw, nword_readstat, pkt_readstat);
  
  status = shift_ir(ftsw, NBIT_IR, IR_CFGOUT, verbose); /* cfg_out */
  printf("status = %03x (%s)", status, status & 0x20 ? "done" : "not-done");
  if (verbose) printf("\n");
  status = shift_dr(ftsw, NBIT_DR, 0, verbose);
  if (! verbose) printf(" %08x\n", status);
  if ((status & statmask) != statexp) {
    printf("bad status\n");
    return -1;
  }
  ret = read_ftsw(ftsw, FTSWREG_JTAGR);
  status = isft2p ? D(ret,28,24) : D(ret,7,3);
  if (status) {
    printf("jtag error %x (%08x)\n", status, ret);
    return -1;
  }

  printf("done.\n");
  return 0;
}
/* ---------------------------------------------------------------------- *\
   idcodeft
\* ---------------------------------------------------------------------- */
int
idcodeft(ftsw_t *ftsw, char *famname)
{
  int i;
  int val;
  xfamily_t *fam;
  
  if (! (fam = findfamily(famname))) {
    printf("no such xilinx device family %s\n", famname);
    exit(1);
  }
  
  nbit_ir   = fam->len;
  code_ir   = fam->idcode;
  printf("code_ir=%x\n", code_ir);
  debugtdo[0] = 0;
  
  reset(ftsw, 3);
  shift_ir(ftsw, nbit_ir, code_ir, 1);
  val = shift_dr(ftsw, nbit_dr, 0, 1);
  write_ftsw(ftsw, FTSWREG_JTAGW, 0);

  for (i=0; xidlist[i].name; i++) {
    if ((val & xidlist[i].mask) == xidlist[i].id) break;
  }
  if (xidlist[i].name) {
    printf("%s found\n", xidlist[i].name);
  } else {
    printf("no known device found\n");
  }
}
/* ---------------------------------------------------------------------- *\
   checkft
\* ---------------------------------------------------------------------- */
void
checkft(ftsw_t *ftsw, int ftswid)
{
  // basic FTSW check
  int cpldid  = read_ftsw(ftsw, FTSWREG_FTSWID);
  int cpldver = read_ftsw(ftsw, FTSWREG_CPLDVER) & 0xffff;
  int fpgaid  = read_ftsw(ftsw, FTSWREG_FPGAID);
  int fpgaver = read_ftsw(ftsw, FTSWREG_FPGAVER) & 0xffff;
  int conf    = read_ftsw(ftsw, FTSWREG_CONF);
  
  if (cpldid != 0x46545357) {
    printf("%s: FTSW#%03d not found: id=%08x\n", PROGRAM, ftswid, cpldid);
    exit(1);
  }
  if (ftswid >= 8 && ftswid < 100 && cpldver < 46) {
    printf("%s: old Spartan-3AN firmware version (%d.%d) found for FTSW2\n",
           PROGRAM, cpldver/100, cpldver%100);
    exit(1);
  }

  if (! (conf & 0x80)) {
    printf("FPGA is not programmed (DONE is not high).\n");
    exit(1);
  }

  switch (fpgaid) {
  case 0x46543255: return; /* FT2U */
  case 0x46543355: return; /* FT3U */
  case 0x4654324f:                     /* FT2O */
  case 0x4654334f:                     /* FT3O */
  case 0x46543250:                     /* FT2P */
  case 0x46543350: isft2p = 1; return; /* FT3P */
  case 0x4a535059: blocktdi = 0; return; /* JSPY */
  default:
    {
      char fpgastr[5];
      int isfpgastr = 1;
      int i;
      for (i=0; i<4; i++) {
	fpgastr[i] = D(fpgaid,(3-i)*8+7,(3-i)*8);
	if (! isprint(fpgastr[i])) isfpgastr = 0;
      }
      fpgastr[4] = 0;
      if (isfpgastr) {
	printf("%s: unsupported FPGA firmware %08x (%s%03d).\n",
               PROGRAM, fpgaid, fpgastr, fpgaver);
      } else {
	printf("%s: unsupported FPGA firmware %08x\n", PROGRAM, fpgaid);
      }
      exit(1);
    }
  }
}
/* ---------------------------------------------------------------------- *\
   devices search
\* ---------------------------------------------------------------------- */
int device_search(ftsw_t *ftsw, int idcode_list[], int listsiz)
{
  int i;
  int ndevs = 0;
  int idcode;
  

  return ndevs;
}
/* ---------------------------------------------------------------------- *\
   device chain
\* ---------------------------------------------------------------------- */
int
device_chain(ftsw_t * ftsw)
{
  int i, j, ndev;
  int ndevs  = 0;
  int differ = 0;
  int idcode = 0;
  int idcode_list[100];
  int listsiz = sizeof(idcode_list)/sizeof(idcode_list[0]);

  reset(ftsw, 3);
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
  tck(ftsw, 0, 0, 0);
  while (ndevs < listsiz) {
    idcode = 0;
    for( i = 0 ; i < 32 ; i ++ ) {
      int bit = tck(ftsw, 0, 1, 0);
      idcode |= (bit<<i);
    }
    if (idcode == 0) break;
    if ((idcode & 0x7fffffff) == 0x7fffffff) break;
    
    idcode_list[ndevs] = idcode & 0x0fffffff;
    ndevs++;
  }
  tck(ftsw, 1, 1, 0);
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);

  do {
    if (idcode == 0 && ndevs == 0) {
      printf("no jtag response, cable or jtag path may be wrong.\n");
    } else if (idcode == 0) {
      printf("no jtag response in the middle of jtag chain.\n");
    } else if (ndevs == 0) {
      printf("no device found in the jtag chain.\n");
    } else if (ndevs == listsiz) {
      printf("too many devices found in the jtag chain (probably noise).\n");
    } else {
      break;
    }
    return -1;
  } while (0);
  
  printf("Number of devices = %d\n", ndevs);
  printf("************* TDO(head) *************\n");
  
  for (ndev=0; ndev < ndevs; ndev++) {
    for (j = 0; xidlist[j].name; j++ ) {
      if ((idcode_list[ndev] & xidlist[j].mask) == xidlist[j].id) {
        break;
      }
    }
    printf("* DEV[%d]: %s (%08x)\n",
           ndev,
           xidlist[j].name ? xidlist[j].name : "unknown device",
           idcode_list[ndev]);
  }
  
  printf("************* TDI(tail) *************\n");

  return 0;
}
/* ---------------------------------------------------------------------- *\
   devices chain
\* ---------------------------------------------------------------------- */
int
devices_chain_yookym(ftsw_t * ftsw)
{
  int i, j, ndev;
  int ndevs = -1;
  int idcode = 0;
  int idcode_list[100];
  
  reset(ftsw, 5);
  reset(ftsw, 5);
  reset(ftsw, 5);
  reset(ftsw, 3);
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
  tck(ftsw, 0, 0, 0);
  while ((idcode & 0x7fffffff) != 0x7fffffff) {
    if (ndevs >= 0) {
      idcode_list[ndevs] = idcode & 0x0fffffff;
    }
    ndevs++;
    idcode = 0;
    for( i = 0 ; i < 32 ; i ++ ) {
      int bit = tck(ftsw, 0, 1, 0);
      idcode |= (bit<<i);
    }
  }
  tck(ftsw, 1, 1, 0);
  tck(ftsw, 1, 0, 0);
  tck(ftsw, 0, 0, 0);
  
  printf("Number of devices = %d\n", ndevs);
  printf("************* TDO(head) *************\n");
  
  for( ndev = 0 ; ndev < ndevs ; ndev++ ) {
    for( j = 0 ; xidlist[j].name ; j++ ) {
      if( ( idcode_list[ndev] & xidlist[j].mask ) == xidlist[j].id ) {
        printf("* DEV[%d]: %s (%08x)\n", ndev,
               xidlist[j].name, idcode_list[ndev] );
      }
    }
  }
  
  printf("************* TDI(tail) *************\n");

  return 0;
}
/* ---------------------------------------------------------------------- * \
   main
\* ---------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
  int offset = -1;
  int value;
  char *p = getenv("FTSW_DEFAULT");
  int id = -1;
  ftsw_t *ftsw;
  int i, ret;
  const char *ARGV0 = argv[0];
  int dodbgtdo = 0;

  printf("jtagft version %s\n", VERSION);

  if (argc < 2 || id < 0 || xjport == 0) {
    printf("usage: %s -<n> [-p<port>] [-h<devices>] [-t<devices>] [command] (<filename>|<fpga-family>)\n",
           ARGV0);
    printf("options:\n");
    printf(" -v         verbose\n");
    printf(" -f         ignore TDO\n");
    printf(" -<n>       select FTSW id\n");
    printf(" -p<port>   FTSW port to use\n");
    printf("commands:\n");
    printf(" idcode    get idcode (default)\n");
    printf(" program   program FPGA (default)\n");
    printf(" chain     print out devices chain\n");
    return 1;
  }

  /* -- open ftsw -- */
  ftsw = open_ftsw(id, FTSW_RDWR);

  if (! ftsw) {
    perror("open");
    printf("FTSW not found.\n");
    exit(1);
  }

  /* -- check ftsw firmware type -- */
  checkft(ftsw, id);

  if (strcmp(argv[1], "program") == 0) {
    progft(ftsw, argv[2]);
  } else if (strcmp(argv[1], "readstat") == 0) {
    readstatft(ftsw, argv[2]);
  } else if (strcmp(argv[1], "chain") == 0) {
    device_chain(ftsw);
  } else {
    if (verbose) {
      printf("head %d %d tail %d %d\n",
	     nbit_head, ndev_head, nbit_tail, ndev_tail);
    }
    idcodeft(ftsw, argv[2]);
    if (dodbgtdo) printf("debugtdo %s\n", debugtdo);
  }
  
  write_ftsw(ftsw, FTSWREG_JTAGW, 1);
}
