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
#include "ft2x012.h"

typedef ft2x_t fast_t;
typedef ft2x_t slow_t;

#include "statft.h"

/* ---------------------------------------------------------------------- *\
   regs2x012
\* ---------------------------------------------------------------------- */
void
regs2x012(ftsw_t *ftsw, struct timeval *tvp, fast_t *f, slow_t *s)
{
  int i;

  gettimeofday(tvp, 0);

  s->ftswid     = read_ftsw(ftsw, FT2X_FTSWID);
  s->cpldver    = read_ftsw(ftsw, FT2X_CPLDVER);
  s->conf       = read_ftsw(ftsw, FT2X_CONF);
  s->clksel     = read_ftsw(ftsw, FT2X_CLKSEL);
  s->fpgaid     = read_ftsw(ftsw, FT2X_FPGAID);
  s->fpgaver    = read_ftsw(ftsw, FT2X_FPGAVER);
  s->clkfreq    = read_ftsw(ftsw, FT2X_CLKFREQ);
  f->utime      = read_ftsw(ftsw, FT2X_UTIME);
  f->ctime      = read_ftsw(ftsw, FT2X_CTIME);
  s->xmask      = read_ftsw(ftsw, FT2X_XMASK);
  s->jctl       = read_ftsw(ftsw, FT2X_JCTL);
  s->jreg       = read_ftsw(ftsw, FT2X_JREG);
  s->jpd        = read_ftsw(ftsw, FT2X_JPD);
  s->jpll       = read_ftsw(ftsw, FT2X_JPLL);
  s->reset      = read_ftsw(ftsw, FT2X_RESET);
  s->rstutim    = read_ftsw(ftsw, FT2X_RSTUTIM);
  s->rstctim    = read_ftsw(ftsw, FT2X_RSTCTIM);
  s->errutim    = read_ftsw(ftsw, FT2X_ERRUTIM);
  s->errctim    = read_ftsw(ftsw, FT2X_ERRCTIM);
  f->errport    = read_ftsw(ftsw, FT2X_ERRPORT);
  f->ttup       = read_ftsw(ftsw, FT2X_TTUP);
  s->handbusy   = read_ftsw(ftsw, FT2X_HANDBUSY);
  f->xalive     = read_ftsw(ftsw, FT2X_XALIVE);
  f->xlinkup    = read_ftsw(ftsw, FT2X_XLINKUP);
  f->xlinkdn    = read_ftsw(ftsw, FT2X_XLINKDN);
  f->xbusy      = read_ftsw(ftsw, FT2X_XBUSY);
  f->xbsyin     = read_ftsw(ftsw, FT2X_XBSYIN);
  f->xerr       = read_ftsw(ftsw, FT2X_XERR);
  f->busy       = read_ftsw(ftsw, FT2X_BUSY);
  for (i=0; i<=19; i++) {
    f->xdath[i]   = read_ftsw(ftsw, FT2X_XDATH + i*4);
  };
  for (i=0; i<=19; i++) {
    f->xdatl[i]   = read_ftsw(ftsw, FT2X_XDATL + i*4);
  };
  for (i=0; i<=19; i++) {
    f->xbcnt[i]   = read_ftsw(ftsw, FT2X_XBCNT + i*4);
  };
  s->xmanual    = read_ftsw(ftsw, FT2X_XMANUAL);
  s->clrdelay   = read_ftsw(ftsw, FT2X_CLRDELAY);
  s->incdelay   = read_ftsw(ftsw, FT2X_INCDELAY);
  s->xackq      = read_ftsw(ftsw, FT2X_XACKQ);
}
/* ---------------------------------------------------------------------- *\
   summary2x012
\* ---------------------------------------------------------------------- */
void
summary2x012(struct timeval *tvp, fast_t *f, slow_t *s)
{
  int clkfreq;
  double tdiff;
  int i;
  
  /* a few things to be calculated beforehand */
  clkfreq = D(s->clkfreq,23,0) | 0x07000000;
  tdiff = tvp->tv_sec + tvp->tv_usec * 1.0e-6
    - f->utime - (double)f->ctime / clkfreq;

  /* only useful registers */
  P("18   xmask=%08x\n",
    s->xmask);

  P("20   reset=%08x%s%s%s%s\n",
    s->reset,
    B(s->reset,29) ? " notagerr" : "",
    B(s->reset,16) ? " regbusy" : "",
    B(s->reset,21) ? " PAUSED" : "",
    B(s->reset,20) ? " RUNNING" : "" );

  P("21/22 trst=%08x %08x src=%d\n",
    s->rstutim,
    D(s->rstctim,26,0),
    D(s->rstctim,31,28));
  
  P("23/24 terr=%08x %08x src=%d\n",
    s->errutim,
    D(s->errctim,26,0),
    D(s->errport,19,0));

  P("28    b2tt=%08x%s%s b=%d%d%d %c%02x\n",
    s->ttup,
    Bs(s->ttup,31," ttup"),
    Bs(s->ttup,30," clkup"),
    B(s->ttup,15),
    D(s->ttup,11,10),
    D(s->ttup,14,12),
    B(s->ttup,8) ? 'K' : 'D',
    D(s->ttup,7,0));

  P("30-32 aliv=%05x linkup=%05x linkdn=%05x\n",
    s->xalive, s->xlinkup, s->xlinkdn);
  P("33-35 busy=%05x  bsyin=%05x    err=%05x\n",
    s->xbusy, s->xbsyin, s->xerr);
  P("3a    busy=%08x%s%s\n",
    s->busy,
    B(s->busy, 28) ? " BSYIN" : Bs(s->busy, 29, "BUSY"),
    B(s->busy, 14) ? " ERRIN" : Bs(s->busy, 15, "ERROR"));
    
  for (i=0; i<19; i++) {
    if (B(s->xlinkup,i)) {
      P("%02x%02x%02x X%02d=%08x %08x %08x",
	0x3c+i, 0x50+i, 0x64+i, i, s->xdath[i], s->xdatl[i], s->xbcnt[i]);
      P("%s%s%s%s%s",
	D(s->xdath[i],27,24)?" err=":"",
	Bs(s->xdath[i],24,"A"),
	Bs(s->xdath[i],25,"B"),
	Bs(s->xdath[i],26,"C"),
	Bs(s->xdath[i],27,"D"));
      P("%s%s%s%s%s",
	(D(s->xdath[i],23,20) || B(s->xdath[i],15)) ?" ful=":"",
	Bs(s->xdath[i],20,"A"),
	Bs(s->xdath[i],21,"B"),
	Bs(s->xdath[i],22,"C"),
	Bs(s->xdath[i],23,"D"),
	Bs(s->xdath[i],15,"L"));
      P("\n");
    }
  }
}
/* ---------------------------------------------------------------------- *\
   stat2x012
\* ---------------------------------------------------------------------- */
void
stat2x012(ftsw_t *ftsw, int ftswid)
{
  struct timeval tv;
  ft2x_t p;
  
  /* retrieve time just before reading registers */
  /* read registers */
  regs2x012(ftsw, &tv, &p, &p);

  statft_assert(&tv, ftswid, p.conf, p.fpgaver, p.utime, p.jpll);
  
  statft_headline(ftswid, p.fpgaid, p.fpgaver, &tv);

  if (verbose)  {
    /* stat2x_verbose(&tv, &p, &p); */
  } else {
    summary2x012(&tv, &p, &p);
  }
}
