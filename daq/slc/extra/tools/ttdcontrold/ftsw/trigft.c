/* ---------------------------------------------------------------------- *\

   trigft.c - FTSW dummy trigger generator

   Mikihiko Nakao, KEK IPNS

   2013071900  first version
   
\* ---------------------------------------------------------------------- */

#define VERSION 2013071900

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include "ftsw.h"

char *ARGV0  = 0;
char *DEVICE = 0;
int verbose = 0;

/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
  char *p = getenv("FTSW_DEFAULT");
  int ftid = p ? atoi(p) : 0;
  ftsw_t *ftsw;
  int i;
  int ret;
  int n=0;
  int m=0;

  int ftswid;
  int fpgaid;
  int fpgaver;

  unsigned long reg_ntrg = 0;
  unsigned long reg_rate = 0;

  int dumtyp = -1;
  double dumrate = 0;
  int dumcnt = -1;
  int rateval = 0;
  int rateexp = 0;
  int rateopt = 0;
  static char *dumstr[] = {
    "none", "i", "tlu", "3", "pulse", "revo", "random", "poisson" };

  while (argc > 1 && argv[1][0] == '-') {
    if (isdigit(argv[1][1])) {
      ftid = atoi(&argv[1][1]);
    }
    argc--, argv++;
  }

  if (argc > 1) {
    static struct { char *name; int val; } tbl[] = {
      { "i",  1 }, { "iack", 1 },
      { "tlu", 2 }, { "aux", 2 },
      { "pulse", 4 }, { "pls", 4 },
      { "revo",  5 }, { "rev", 5 },
      { "random", 6 }, { "rnd", 6 },
      { "poisson", 7 }, { "poi", 7 },
      { "one", 8 },
      { "stop", 9 } };
    for (i=0; i<sizeof(tbl)/sizeof(tbl[0]); i++) {
      if (strcmp(argv[1], tbl[i].name) == 0) break;
    }
    if (i == sizeof(tbl)/sizeof(tbl[0])) {
      argc = 0;
    } else {
      dumtyp = tbl[i].val;
      argc--, argv++;
    }

    if (dumtyp == 8 || dumtyp == 9) {
      dumtyp = 4;
      dumcnt = (dumtyp == 8) ? 1 : 0;
      dumrate = 302000.0;
    }
  }

  if (argc > 1 && dumrate == 0 && dumtyp >= 4) {
    dumrate = atof(argv[1]);
    argc--, argv++;
  }

  if (argc > 1 && dumcnt < 0) {
    dumcnt = atoi(argv[1]);
    argc--, argv++;
  }

  if (dumtyp <= 0 || dumrate == 0) {
    printf("usage: trigft -<ftswid> <type> [<rate>] [<count>] [<pos>]\n");
    printf("- type is one of: i tlu pulse revo random poisson one stop\n");
    printf("- type=one  equivalent to pulse at high rate with count=1\n");
    printf("- type=stop equivalent to pulse at high rate with count=0\n");
    printf("- rate is needed for pulse, revo, random, poisson\n");
    printf("- if count is not given, no limit on number of triggers\n");
    printf("- if type=revo, pos is the position within revolution cycle\n");
    printf("- default value of pos=0\n");
    return 1;
  }

  if (dumtyp >= 4 && dumtyp <= 7) {
    /* rate = 127216000 / ( 30 * (14 + val * pow(2, exp)) + 1 )
       30 * (14 + valpow) + 1 = 127216000 / rate
       valpow = ((127216000 / rate) - 1) / 30 - 14 */
    double valpow = 0;
    double newrate = 0;
    
    switch (dumtyp) {
    case 4: valpow = (127216000 / dumrate - 1) / 30 - 14; break;
    case 5: valpow = (127216000 / dumrate) / 1280;        break;
    /* valpow for random / poisson is not an exact value */
    case 6: valpow = (127216000 / dumrate) / 512;         break;
    case 7: valpow = (127216000 / dumrate) / 68;          break;
    }
    
    if (valpow <= 0) {
      printf("invalid rate %5.3f Hz\n", dumrate);
      return 1;
    }
    
    rateexp = log(valpow / 1024) / log(2) + 1;
    if (rateexp <  0) rateexp = 0;
    if (rateexp > 15) rateexp = 15;
    rateval = valpow / pow(2, rateexp) - (dumtyp == 4 ? 0 : 1);
    if (rateval <    0) rateval = 0;
    if (rateval > 1023) rateval = 1023;

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

    printf("%s trigger rate %5.3f Hz (n=%d exp=%d)\n",
	   dumstr[dumtyp], newrate, rateval, rateexp);

  }

  reg_ntrg = dumcnt;
  reg_rate =
    ((dumtyp  & 0x007) << 0) |
    ((rateexp & 0x00f) << 4) |
    ((rateval & 0x3ff) << 8) |
    ((rateopt & 0xfff) << 20);

  
  ftsw = open_ftsw(ftid, FTSW_RDWR);
  
  if (! ftsw) {
    perror("open");
    printf("VME cannot be accessed.\n");
    exit(1);
  }

  ftswid  = read_ftsw(ftsw, FTSWREG_FTSWID);
  fpgaid  = read_ftsw(ftsw, FTSWREG_FPGAID);
  fpgaver = read_ftsw(ftsw, FTSWREG_FPGAVER);
  if (ftswid != 0x46545357) {
    printf("FTSW #%03d is not found.\n");
    return 0;
  } else if ((fpgaid == 0x46543255 && fpgaver >= 17) ||  /* FT2U */
	     (fpgaid == 0x46543355 && fpgaver >= 17)) {  /* FT3U */
    /* OK */
  } else {
    printf("FTSW #%03d has an incompatible firmware version.\n");
    return 0;
  }
  
  write_ftsw(ftsw, FTSWREG_TLIMIT, reg_ntrg);
  write_ftsw(ftsw, FTSWREG_TRGSET, reg_rate);
  
  
#if 0
	   rateval, rateexp );
    }
  } else if (dumtyp == 5) {
    /* rate = 127216000 / [ 1280 * ( 1 + val ) * pow(2, exp) ] */
    double 
    if (valpow <= 0) {
      printf("revo trigger invalid rate %5.3f Hz\n", dumrate);
    } else {
      rateexp = log(valpow / 1024) / log(2) + 1;
      if (rateexp <  0)  rateexp = 0;
      if (rateexp >= 16) rateexp = 15;
      rateval = valpow / pow(2, rateexp) - 1;
      if (rateval < 0)    rateval = 0;
      if (rateval > 1023) rateval = 1023;
      printf("revo trigger rate %5.3f Hz (n=%d exp=%d)\n", 
	     127216000 / ( 1280 * (1 + rateval * pow(2, rateexp)) ),
	     rateval, rateexp );
    }
      
      
  }
#endif
#if 0  
  
  ftsw = open_ftsw(ftid, FTSW_RDWR);
  
  if (! ftsw) {
    perror("open");
    printf("FTSW is not found.\n");
    exit(1);
  }

  gettimeofday(&tv, 0);
  sec = tv.tv_sec;

  for (n=0; ; n++) {
    gettimeofday(&tv, 0);
    if (sec != tv.tv_sec) {
      write_ftsw(ftsw, 0x120>>2, tv.tv_sec);
      return 0; 
      for (m=0; ; m++) {
	gettimeofday(&tv2, 0);
	if (tv.tv_usec != tv2.tv_usec) break;
      }
      printf("n=%d sec=%ld usec=%ld m=%d delta=%ld\n",
	     n, tv.tv_sec, tv.tv_usec, m, tv2.tv_usec);
      break;
    }
  }
#endif
}
