
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
   20160217  ft3t (temporary ft3r for TOP)
   20160330  update for statft_nsm compatibility
   20160423  curses version
   20160426  -DCURSES
   
\* ---------------------------------------------------------------------- */

int VERSION = 20160426;

#define STATFT_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
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

int verbose = 0;
int jitter = 0;
int interval = 0;

#include "statft.h"

/* ---------------------------------------------------------------------- *\
   statft_headline
\* ---------------------------------------------------------------------- */
void
statft_headline(int ftswid, int fpgaid, int fpgaver, struct timeval *tvp, char* ss)
{
  struct tm *tp = localtime(&tvp->tv_sec);
  char build[3];
  if (D(fpgaver,15,10)) {
    sprintf(build, "%c", 'a'+D(fpgaver,15,10)-1);
  } else {
    *build = 0;
  }
  char css[100];
  sprintf(css, "%s %d %s #%03d / %c%c%c%c%03d%s - %04d.%02d.%02d %02d:%02d:%02d.%03d\n",
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
  strcat(ss, css);
}

/* ---------------------------------------------------------------------- *\
   localtimestr
\* ---------------------------------------------------------------------- */

const char * localtimestr(int utim)
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
const char * tdiffstr(int tdiff)
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
const char * dumtrgstr(int rate)
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
    return buf;
  } else {
    return dumstr[dumtyp];
  }
}

/* ---------------------------------------------------------------------- *\
   statft_jitter
\* ---------------------------------------------------------------------- */
void statft_jitter(ftsw_t *ftsw, int reg_jctl, int reg_jreg, char* ss)
{
  int i;
  int jreg[3];
  for (i=0; i<3; i++) {
    write_ftsw(ftsw, reg_jctl, 0x0e + i*0x20);
    usleep(1);
    jreg[i] = read_ftsw(ftsw, reg_jreg);
  }
  char css[100];
  sprintf(css, "%02x   jreg0=%08x jreg1=%08x jreg2=%08x\n",
	  FTSWADDR(reg_jctl), jreg[0], jreg[1], jreg[2]);
  strcat(ss,css);
}

