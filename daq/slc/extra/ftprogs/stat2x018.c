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
#include "ft2x018.h"

typedef ft2x_t fast_t;
typedef ft2x_t slow_t;

#include "statft.h"

/* ---------------------------------------------------------------------- *\
   regs2x018
\* ---------------------------------------------------------------------- */
void
regs2x018(ftsw_t *ftsw, struct timeval *tvp, fast_t *f, slow_t *s)
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
  s->myaddr     = read_ftsw(ftsw, FT2X_MYADDR);
  s->jctl       = read_ftsw(ftsw, FT2X_JCTL);
  s->jpd        = read_ftsw(ftsw, FT2X_JPD);
  s->jpll       = read_ftsw(ftsw, FT2X_JPLL);
  s->errutim    = read_ftsw(ftsw, FT2X_ERRUTIM);
  s->errctim    = read_ftsw(ftsw, FT2X_ERRCTIM);
  f->errport    = read_ftsw(ftsw, FT2X_ERRPORT);
  f->errbit     = read_ftsw(ftsw, FT2X_ERRBIT);
  f->ttup       = read_ftsw(ftsw, FT2X_TTUP);
  s->regdbg     = read_ftsw(ftsw, FT2X_REGDBG);
  f->xbusy      = read_ftsw(ftsw, FT2X_XBUSY);
  f->xbsyin     = read_ftsw(ftsw, FT2X_XBSYIN);
  f->xerr       = read_ftsw(ftsw, FT2X_XERR);
  f->xlinkdn    = read_ftsw(ftsw, FT2X_XLINKDN);
  f->xalive     = read_ftsw(ftsw, FT2X_XALIVE);
  f->xlinkup    = read_ftsw(ftsw, FT2X_XLINKUP);
  s->lckfreq    = read_ftsw(ftsw, FT2X_LCKFREQ);
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
}
/* ---------------------------------------------------------------------- *\
   summary2x018
\* ---------------------------------------------------------------------- */
void
summary2x018(struct timeval *tvp, fast_t *f, slow_t *s)
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

  P("30-32 busy=%05x  bsyin=%05x    err=%05x\n",
    s->xbusy, s->xbsyin, s->xerr);
  P("33-35 aliv=%05x linkup=%05x linkdn=%05x\n",
    s->xalive, s->xlinkup, s->xlinkdn);
  P("3a    busy=%08x%s%s\n",
    s->busy,
    B(s->busy, 28) ? " BSYIN" : Bs(s->busy, 29, "BUSY"),
    B(s->busy, 14) ? " ERRIN" : Bs(s->busy, 15, "ERROR"));
    
  for (i=0; i<19; i++) {
    if (B(s->xlinkup,i)) {
      P("%02x%02x%02x X%02d=%08x %08x %08x",
	0x3c+i, 0x50+i, 0x64+i, i, s->xdath[i], s->xdatl[i], s->xbcnt[i]);
      P("en=%s%s%s%s%s empty=%s%s%s%s%s",
        Bs(s->xdath[i],28,"A"),
        Bs(s->xdath[i],29,"B"),
        Bs(s->xdath[i],30,"C"),
        Bs(s->xdath[i],31,"D"),
        D(s->xdath[i],31,28)?"":"none",
        Bs(s->xdath[i],16,"A"),
        Bs(s->xdath[i],17,"B"),
        Bs(s->xdath[i],18,"C"),
        Bs(s->xdath[i],19,"D"),
        D(s->xdath[i],19,16)?"":"none");
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
   stat2x018
\* ---------------------------------------------------------------------- */
void
stat2x018(ftsw_t *ftsw, int ftswid)
{
  struct timeval tv;
  ft2x_t p;
  
  /* retrieve time just before reading registers */
  /* read registers */
  regs2x018(ftsw, &tv, &p, &p);

  statft_headline(ftswid, p.fpgaid, p.fpgaver, &tv);

  statft_assert(&tv, ftswid, p.conf, p.fpgaver, p.utime, p.jpll);
  
  if (verbose)  {
    /* stat2x_verbose(&tv, &p, &p); */
  } else {
    summary2x018(&tv, &p, &p);
  }
}
