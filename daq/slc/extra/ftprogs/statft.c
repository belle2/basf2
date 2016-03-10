
/* ---------------------------------------------------------------------- * \

   statft.c - FTSW status

   Mikihiko Nakao, KEK IPNS

   20131207  new
   20131218  3a/3b update (for ft2u041)
   20131226  small update
   20131227  merged with pocket_ttd, ignoretime
   20140102  color option, date at the top
   20140105  exprun fix, include deadtime
   20140106  tludelay (ft2u044), green
   20140107  dumstr fix: tlu is 2, not 3
   20140108  reset bits, 29-2c fix
   20140109  busy to red
   20140116  error to magenta
   20140117  status line
   20140118  status line tuning, timtlu fix
   20140403  fpgaver in the first line
   20140408  dumponly when dump, stat3f entry
   20140409  ft3f->sta
   20140415  sort out around 39-3b
   20140419  autodump handling
   20140708  no prefix fee/copper name
   20140718  fix last/out address
   20140806  treat FT3U as FT2U
   20140812  treat FT2P as FT2U
   20140817  ft2p / ft2x
   20140822  stat2p update
   20140826  show unknown fpga type in ascii if printable
   20140913  show version for unknown fpga type, too, if printable
   20140930  ft3d012
   20141006  ft3d update
   20141012  ft2p color
   20141016  ft2p mod
   20141029  ft2p fix
   20141031  max ftswid to be 256
   20141218  ft2p014 update
   20141219  ft3p as ft2p
   20141225  dump3d
   20150114  ft2x018
   20150209  fix ft2p odata/b/c address
   20150309  statft.h, ft2u067
   20150317  stat2d020.c stata,b,c address fix
   20150318  stat2p014.c cntrst -> errsrc
   20150326  stat2p014.c errport -> errsrc
   20150327  stat2p014.c run start time
   20150409  color2p to color2u
   20150531  stat2p014.c for ft3p027
   20150604  red busy only when tout is not updated
   20150730  ft2o/ft3o
   20150908  tagerr, fifoerr, seu only when ttup
   20160106  clksrc
   
\* ---------------------------------------------------------------------- */

int VERSION = 20160106;

#define STATFT_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include "ftsw.h"

#ifdef POCKET_TTD
extern void xprint0(const char *fmt, ...);
extern void xprint1(const char *fmt, ...);
#define P xprint0
#define PP xprint1
int doassert = 0;

/*
typedef unsigned char uchar;
typedef int16_t int16;
typedef int32_t int32;
*/

#else /* ! POCKET_TTD */

char *PROGRAM  = 0;
int verbose = 0;
int jitter = 0;
int doassert = 1;
int interval = 0;
int showjitter = 0;
int showdump = 0;
int showcolor = 0;

/*#define P printf("* ");printf*/
#define P printf
#define PP printf
#endif /* POCKET_TTD */

#include "statft.h"
#include "ftstat.h"

extern ftstat_t g_ftstat;

/* ---------------------------------------------------------------------- * \
   statft_assert
\* ---------------------------------------------------------------------- */
void
statft_assert(struct timeval *tvp, int id,
	      int conf, int fpgaver, int utime, int jsta)
{
  int isjsta = (D(jsta,27,0) == 0 ? 0 : 1);
  /* asserts */
  if (B(conf, 7) == 0) {
    printf("assert: fpga is not programmed\n");
    if (doassert) exit(1);
  }
  if (D(fpgaver,31,24) != 3 || D(fpgaver,23,16) != id) {
    printf("assert: incorrect VME base address %04x for id=%d\n",
           D(fpgaver,31,16), id);
    if (doassert) exit(1);
  }
  if (utime < 0x100000) {
    printf("warning: time is not yet set\n");
  } else if (abs(tvp->tv_sec - utime) > 1) {
    printf("warning: time difference = %d sec\n", (int)(tvp->tv_sec - utime));
  }
  if (isjsta && (D(jsta,27,24) != 0x0c || D(jsta,15,0) != 0x8000)) {
    printf("assert: no stable clock input (jsta=%08x)\n", jsta);
    if (doassert) exit(1);
  }
}
/* ---------------------------------------------------------------------- *\
   statft_headline
\* ---------------------------------------------------------------------- */
void
statft_headline(int ftswid, int fpgaid, int fpgaver, struct timeval *tvp)
{
  struct tm *tp = localtime(&tvp->tv_sec);
  char build[3];
#ifndef POCKET_TTD
  if (showcolor) printf("\033[37;1m");
#endif
  if (D(fpgaver,15,10)) {
    sprintf(build, "%c", 'a'+D(fpgaver,15,10)-1);
  } else {
    *build = 0;
  }
  sprintf(g_ftstat.statft, "%s %d %s #%03d / %c%c%c%c%03d%s - %04d.%02d.%02d %02d:%02d:%02d.%03d\n",
    "statft version",
    VERSION,
    "FTSW",
    ftswid,
    tolower(D(fpgaid, 31, 24)),
    tolower(D(fpgaid, 23, 16)),
    tolower(D(fpgaid, 15,  8)),
    tolower(D(fpgaid,  7,  0)),
    D(fpgaver, 9, 0),
    build,
    tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday,
    tp->tm_hour, tp->tm_min, tp->tm_sec, (int)(tvp->tv_usec/1000));
  
}
/* ---------------------------------------------------------------------- *\
   statft_jitter
\* ---------------------------------------------------------------------- */
void
statft_jitter(ftsw_t *ftsw, int reg_jctl, int reg_jreg)
{
  int i;
  int jreg[3];
  for (i=0; i<3; i++) {
    write_ftsw(ftsw, reg_jctl, 0x0e + i*0x20);
    usleep(1);
    jreg[i] = read_ftsw(ftsw, reg_jreg);
  }
  P("%02x   jreg0=%08x jreg1=%08x jreg2=%08x\n",
    FTSWADDR(reg_jctl), jreg[0], jreg[1], jreg[2]);
    
  ASSERT("jreg0", jreg[0] != 0x4a200d50);
  ASSERT("jreg1", (jreg[1] & 0x0fffffff) != 0x03874061);
}
/* ---------------------------------------------------------------------- *\
   localtimestr
\* ---------------------------------------------------------------------- */
const char *
localtimestr(int utim)
{
  static int i = 0;
  static char buf[4][256];
  time_t now = (time_t)utim;
  struct tm *t = localtime(&now);
  static const char *m[12] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
  static const char *w[7] = {     "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
  i %= 4;
  sprintf(buf[i], "%s %s %2d %02d:%02d:%02d %4d",
	  w[t->tm_wday], m[t->tm_mon],
	  t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, t->tm_year+1900);
  return buf[i++];
}
/* ---------------------------------------------------------------------- *\
   tdiffstr
\* ---------------------------------------------------------------------- */
const char *
tdiffstr(int tdiff)
{
  static int i = 0;
  static char buf[4][256];

  i %= 4;
  if (tdiff >= 3600*100) {
    sprintf(buf[i], "%3.1fd", tdiff/(3600*24.0));
  } else if (tdiff > 60*300) {
    sprintf(buf[i], "%3.1fh", tdiff/(3600.0));
  } else if (tdiff > 60*5) {
    sprintf(buf[i], "%3.1fm", tdiff/(60.0));
  } else {
    sprintf(buf[i], "%ds", tdiff);
  }
  return buf[i++];
}
/* ---------------------------------------------------------------------- *\
   dumptrgstr
\* ---------------------------------------------------------------------- */
const char *
dumtrgstr(int rate)
{
  int dumtyp  = (rate >>  0) & 0x007;
  int rateexp = (rate >>  4) & 0x00f;
  int rateval = (rate >>  8) & 0x3ff;
  int rateopt = (rate >> 20) & 0xfff;
  static char buf[128];
  static char *dumstr[] = {
    "none", "aux", "i", "tlu", "pulse", "revo", "random", "poisson" };

  if (dumtyp >= 4 && dumtyp <= 7) {
    double newrate = 0;

    switch (dumtyp) {
    case 4:
      newrate = 127216000 / ( 30 * (14 + rateval * pow(2, rateexp)) + 1);
      break;
    case 5:
      newrate = 127216000 / ( 1280 * (1 + rateval * pow(2, rateexp)) );
      break;
    case 6:
      newrate = 127216000 / ( 512 * (1 + rateval * pow(2, rateexp)) );
      break;
    case 7:
      newrate = 127216000 / ( 68  * (1 + rateval * pow(2, rateexp)) );
      break;
    }

    sprintf(buf, "%s trigger rate %5.3f Hz (n=%d exp=%d)",
	    dumstr[dumtyp], newrate, rateval, rateexp);
    g_ftstat.rateall = newrate;
    return buf;
  } else {
    return dumstr[dumtyp];
  }
}
/* ---------------------------------------------------------------------- *\
   statft_dump
\* ---------------------------------------------------------------------- */
void
statft_dump(int dumpk, int dumpi, int dumpo[], int dump2[])
{
  int i;
  int j;
  int dump[32];
  int found;

  /* -- 88..99 -- */
  
  P("dumpk=%08x dumpi=%08x\n", dumpk, dumpi);
  
  for (i=0; i<8; i++) {
    for (j=0; j<4; j++) {
      dump[i*4 + j] = D(dumpo[i], j*8+7, j*8);
    }
  }
  
  for (found=31; found>=0; found--) {
    if (dumpk & (1<<found) && dump[found] == 0x3c) break;
    if (dumpk & (1<<found) && dump[found] == 0xbc && found) {
      found--;
      break;
    }
  }
  WARN("frame boundary not found", found <= 15);

  for (i=0; i<(found-15)%16; i++) PP("    ");
  for (i=31; i>=0; i--) {
    if (i == found && i != 31) PP("\n");
    if (i == found - 16)       PP("\n");
    PP(" %c%02x", dumpk & (1<<i) ? 'K' : 'D', dump[i]);
  }
  PP("\n");

  {
    char dumpa[128];
    char dumpb[1024];
    char dumpc[1024];
    char bit[16][5];
    char *p;
    char *q;
    
    dumpa[0] = dumpb[0] = 0;
    for (i=9; i>=0; i--) {
      sprintf(dumpa+strlen(dumpa), "%08x", dump2[i]);
    }
    for (i=0; i<16; i++) {
      sprintf(bit[i], "%d%d%d%d", (i>>3)&1, (i>>2)&1, (i>>1)&1, i&1);
    }
    for (i=0; i<80; i++) {
      int hex = dumpa[i] >= 'a' ? dumpa[i] - 'a' + 10 : dumpa[i] - '0';
      strcat(dumpb, bit[hex]);
    }
    strcpy(dumpc, dumpb);
    p = strstr(dumpb, "0011111001");
    q = strstr(dumpb, "1100000110");
    if (p || q) {
      if (! p || (q && q < p)) p = q;
      if (p + 170 <= dumpb + 320 &&
	  (strncmp(p+160, "0011111001", 10) == 0 ||
	   strncmp(p+160, "1100000110", 10) == 0)) {
	for (i=0; i<160-(p-dumpb); i++) {
	  dumpc[i] = 'x';
	}
	strcpy(dumpc+(160-(p-dumpb)), dumpb);
      } else {
	PP("%d %10.10s %10.10s\n", (int)(dumpb+320-(p+170)), p+0, p+160);
      }
    }
    for (i=0; i<=strlen(dumpc)-10; i+=10) {
      char buf[16];
      int k=2;
      strcpy(buf, "00");
      strncpy(buf+2, dumpc+i, 2);
      buf[4] = 0;
      PP(" ");
      while (k <= 10) {
	for (j=0; j<16; j++) if (strcmp(buf, bit[j]) == 0) break;
	if (j == 16) {
	  PP("x");
	} else {
	  PP("%1x", j);
	}
	strncpy(buf, dumpc+i+k, 4);
	k += 4;
      }
      if ((i % 160) == 150) PP("\n");
    }
    PP("\n");
  }
}
/* ---------------------------------------------------------------------- *\
   setcolor
\* ---------------------------------------------------------------------- */
#ifndef POCKET_TTD
void
setcolor(int color, const char *text)
{
  char buf[256];
  if (color == 3 || color == 6 || color == 7) {
    printf("\033[4%d;30;1m", color);
  } else {
    printf("\033[4%d;37;1m", color);
  }
  sprintf(buf, "-- %s ", text);
  strcat(buf, "--------------------------------------");
  strcat(buf, "--------------------------------------");
  strcpy(buf+79, "\n");
  printf("%s", buf);
  printf("\033[m");
  printf("\033[3%d;1m", color);
}
#endif
/* ---------------------------------------------------------------------- *\
   stat2u
\* ---------------------------------------------------------------------- */
#ifndef POCKET_TTD
#endif
/* ---------------------------------------------------------------------- *\
   statft
\* ---------------------------------------------------------------------- */
#ifndef POCKET_TTD
void
statft(ftsw_t *ftsw, int ftswid)
{
  // basic FTSW check
  int cpldid  = read_ftsw(ftsw, FTSWREG_FTSWID);
  int cpldver = read_ftsw(ftsw, FTSWREG_CPLDVER) & 0xffff;
  int fpgaid  = read_ftsw(ftsw, FTSWREG_FPGAID);
  int isft2   = ((fpgaid & 0x100) == 0);
  int isft3   = ((fpgaid & 0x100) != 0);
  int ftidhi  = fpgaid & 0xfffffe00;
  int ftidlo  = fpgaid & 0xff;
  int fpgaver = read_ftsw(ftsw, FTSWREG_FPGAVER) & 0x3ff;
  int conf    = read_ftsw(ftsw, FTSWREG_CONF);
  
  if (cpldid != 0x46545357) {
    printf("%s: FTSW#%03d not found: id=%08x\n", PROGRAM, ftswid, cpldid);
    return;
  }
  if (ftswid >= 8 && ftswid < 100 && cpldver < 46) {
    printf("%s: old Spartan-3AN firmware version (%d.%d) found for FTSW2\n",
           PROGRAM, cpldver/100, cpldver%100);
    return;
  }

  if (! (conf & 0x80)) {
    P("FPGA is not programmed (DONE is not high).\n");
    return;
  }

  if ( ftidhi != 0x46543200 ) {
    ftidhi = 0;
  } else if ( ftidlo == 'U' && fpgaver >= 67 ) { /* FTxU */
    stat2u067(ftsw, ftswid);
  } else if ( ftidlo == 'U' && fpgaver >= 41 ) {
    stat2u041(ftsw, ftswid);
  } else if ( ftidlo == 'O' && fpgaver >= 26 ) { /* FTxO */
    stat2p026(ftsw, ftswid, showcolor);
  } else if ( ftidlo == 'P' && fpgaver >= 26 ) { /* FTxP */
    stat2p026(ftsw, ftswid, showcolor);
  } else if ( ftidlo == 'P' && fpgaver >= 14 ) { /* FTxP */
    stat2p014(ftsw, ftswid, showcolor);
  } else if ( ftidlo == 'P' && fpgaver >=  2 ) {
    stat2p002(ftsw, ftswid, showcolor);
  } else if ( ftidlo == 'X' && fpgaver >= 18 ) { /* FTxX */
    stat2x018(ftsw, ftswid);
  } else if ( ftidlo == 'X' && fpgaver >= 12 ) {
    stat2x012(ftsw, ftswid);
  } else if ( (ftidlo == 'D' || ftidlo == 'R') ) { /* FTxD, FTxR */
    if (fpgaver >= 20 ) {
      stat2d020(ftsw, ftswid);
    } else if (fpgaver >= 12 && isft2) {
      stat2d012(ftsw, ftswid);
    } else if (fpgaver >= 12 && isft3) {
      stat3d012(ftsw, ftswid);
    } else if (fpgaver >= 6  && isft3) {
      stat3d006(ftsw, ftswid);
    } else {
      ftidhi = 0;
    }
  } else if ( ftidlo == 'F' ) { /* FTxF */
    stat3f001(ftsw, ftswid);
  }

  if (ftidhi == 0) {
    char fpgastr[5];
    int isfpgastr = 1;
    int i;
    for (i=0; i<4; i++) {
      fpgastr[i] = D(fpgaid,(3-i)*8+7,(3-i)*8);
      if (! isprint(fpgastr[i])) isfpgastr = 0;
    }
    fpgastr[4] = 0;
    if (isfpgastr) {
      P("unsupported FPGA firmware %08x (%s%03d) is programmed.\n",
        fpgaid, fpgastr, fpgaver);
    } else {
      P("unknown FPGA firmware %08x is programmed.\n", fpgaid);
    }
  }
}
#endif /* ! POCKET_TTD */
/* ---------------------------------------------------------------------- *\
   handler
\* ---------------------------------------------------------------------- */
#ifndef POCKET_TTD
void
handler(int sig)
{
  printf("\033[m");
  exit(0);
}
#endif
/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
#ifndef POCKET_TTD
#ifdef POCKET_TTD
int
main(int argc, char **argv)
{
  int readonly = 0;
  int offset = -1;
  int value;
  char *p = getenv("FTSW_DEFAULT");
  int ftswid = p ? atoi(p) : -1;
  ftsw_t *ftsw;

  PROGRAM = argv[0];

  while (argc > 1) {
    if (argv[1][0] == '-') {
      if (isdigit(argv[1][1])) {
	ftswid = atoi(&argv[1][1]);
      } else if (argv[1][1] == 'v') {
	verbose = 1;
      } else if (argv[1][1] == 'j') {
	showjitter = 1;
      } else if (argv[1][1] == 'd') {
	showdump = 1;
      } else if (argv[1][1] == 'a') {
	doassert = 0;
      } else if (argv[1][1] == 'i') {
	interval = isdigit(argv[1][2]) ? atoi(&argv[1][2]) : 2;
	if (interval <= 0) interval = 2;
      } else if (argv[1][1] == 'c') {
	showcolor = 1;
        if (! interval) {
          interval = isdigit(argv[1][2]) ? atoi(&argv[1][2]) : 2;
          if (interval <= 0) interval = 2;
        }
      }
    } else if (isdigit(argv[1][0])) {
      ftswid = atoi(&argv[1][0]);
    }
    argc--, argv++;
  }
  if (ftswid < 0 || ftswid >= 256) {
    printf("usage: statft [-v] [-]<n>\n");
    printf("options:\n");
    printf(" -<n>    select <n>-th FTSW (<n>=1,2,3,4)\n");
    printf(" -v      verbose (all but following slow registers are shown)\n");
    printf(" -j      jitter  (slow jitter registers are shown)\n");
    printf(" -d      dump    (slow dump registers are shown)\n");
    printf(" -a      ignore any errors\n");
    printf(" -i[<n>] repeat every n(default=2) seconds\n");
    printf(" -c[<n>] repeat with some color\n");
    return 1;
  }

  ftsw = open_ftsw(ftswid, FTSW_RDWR);
  
  if (! ftsw) {
    /* perror("open"); */
    printf("VMEbus is not found.\n");
    exit(1);
  }

  if (interval) {
    signal(SIGINT,  handler);
    signal(SIGTERM, handler);
    signal(SIGSEGV, handler);
    signal(SIGHUP,  handler);
    signal(SIGUSR1, handler);
    signal(SIGUSR2, handler);
    while (1) {
      printf("\033[H\033[2J");
      statft(ftsw, ftswid);
      sleep(interval);
    }
  } else {
    statft(ftsw, ftswid);
    printf("%04d.%06d.%04d\n", g_ftstat.exp, 
	   g_ftstat.run,
	   g_ftstat.sub);
    printf("%d -> %d -> %d\n", g_ftstat.tincnt, 
	   g_ftstat.atrigc,
	   g_ftstat.toutcnt);
    printf("%f -> %f -> %f\n", g_ftstat.rateall, 
	   g_ftstat.raterun,
	   g_ftstat.rateout);
    printf("%s\n%s\n%s\n%s\n%s\n", g_ftstat.busy, 
	   g_ftstat.reset,
	   g_ftstat.stafifo,
	   g_ftstat.err,
	   g_ftstat.errport);
  }

  return 0;
}
#endif /* ! POCKET_TTD */
#endif /* ! POCKET_TTD */
