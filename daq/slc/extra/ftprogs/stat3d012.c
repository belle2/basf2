/*
  stat3d012.c
  
  20140930 first version
  20141006 BSYIN => BUSY
 */

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
#include "ft3d012.h"

typedef ft3d_t fast_t;
typedef ft3d_t slow_t;

#include "statft.h"

/* ---------------------------------------------------------------------- *\
   regs3d012
\* ---------------------------------------------------------------------- */
void
regs3d012(ftsw_t *ftsw, struct timeval *tvp, fast_t *f, slow_t *s)
{
  int i;

  gettimeofday(tvp, 0);

  s->ftswid     = read_ftsw(ftsw, FT3D_FTSWID);
  s->cpldver    = read_ftsw(ftsw, FT3D_CPLDVER);
  s->conf       = read_ftsw(ftsw, FT3D_CONF);
  s->clksel     = read_ftsw(ftsw, FT3D_CLKSEL);
  s->fpgaid     = read_ftsw(ftsw, FT3D_FPGAID);
  s->fpgaver    = read_ftsw(ftsw, FT3D_FPGAVER);
  f->utime      = read_ftsw(ftsw, FT3D_UTIME);
  f->ctime      = read_ftsw(ftsw, FT3D_CTIME);
  s->omask      = read_ftsw(ftsw, FT3D_OMASK);
  s->reg3s      = read_ftsw(ftsw, FT3D_REG3S);
  s->jpd        = read_ftsw(ftsw, FT3D_JPD);
  s->jpll       = read_ftsw(ftsw, FT3D_JPLL);
  f->ttup       = read_ftsw(ftsw, FT3D_TTUP);
  f->trgtag     = read_ftsw(ftsw, FT3D_TRGTAG);
  f->trgtyp     = read_ftsw(ftsw, FT3D_TRGTYP);
  s->errutim    = read_ftsw(ftsw, FT3D_ERRUTIM);
  s->errctim    = read_ftsw(ftsw, FT3D_ERRCTIM);
  f->errport    = read_ftsw(ftsw, FT3D_ERRPORT);
  s->handbusy   = read_ftsw(ftsw, FT3D_HANDBUSY);
  f->busy       = read_ftsw(ftsw, FT3D_BUSY);
  f->obsyin     = read_ftsw(ftsw, FT3D_OBSYIN);
  f->oerrin     = read_ftsw(ftsw, FT3D_OERRIN);
  f->olinkdn    = read_ftsw(ftsw, FT3D_OLINKDN);
  f->oalive     = read_ftsw(ftsw, FT3D_OALIVE);
  f->olinkup    = read_ftsw(ftsw, FT3D_OLINKUP);
  f->b2ldn      = read_ftsw(ftsw, FT3D_B2LDN);
  f->plldn      = read_ftsw(ftsw, FT3D_PLLDN);
  for (i = 0; i < FT3D_NUMO; i++) {
    f->odata[i]   = read_ftsw(ftsw, FT3D_ODATA + i*4);
  };
  for (i = 0; i < FT3D_NUMO; i++) {
    f->odatb[i]   = read_ftsw(ftsw, FT3D_ODATB + i*4);
  };
  for (i = 0; i < FT3D_NUMO; i++) {
    f->odatc[i]   = read_ftsw(ftsw, FT3D_ODATC + i*4);
  };
  s->omanual    = read_ftsw(ftsw, FT3D_OMANUAL);
  for (i = 0; i < 8; i++) {
    s->dumpo[i]    = read_ftsw(ftsw, FT3D_DUMPO + i*4);
  };
  s->dumpk      = read_ftsw(ftsw, FT3D_DUMPK);
  s->dumpi      = read_ftsw(ftsw, FT3D_DUMPI);
  s->oackq      = read_ftsw(ftsw, FT3D_OACKQ);
  for (i = 0; i < 10; i++) {
    s->dumpb[i]   = read_ftsw(ftsw, FT3D_DUMPB + i*4);
  };
  s->selila     = read_ftsw(ftsw, FT3D_SELILA);
}
/* ---------------------------------------------------------------------- *\
   summary3d012
\* ---------------------------------------------------------------------- */
void
summary3d012(struct timeval *tvp, fast_t *f, slow_t *s)
{
  int i;
  
  /* only useful registers */
  P("18   omask=%08x\n",
    s->omask);

  P("20    b2tt=%08x%s%s b=%d%d%d %c%02x\n",
    s->ttup,
    Bs(s->ttup,31," ttup"),
    Bs(s->ttup,30," clkup"),
    B(s->ttup,15),
    D(s->ttup,11,10),
    D(s->ttup,14,12),
    B(s->ttup,8) ? 'K' : 'D',
    D(s->ttup,7,0));

  P("21/22  tag=%08x typ=%08x tag=%d typ=%d\n",
    s->trgtag,
    s->trgtyp,
    s->trgtag,
    D(s->trgtyp,31,28));
  
  P("23/24 terr=%08x %08x src=%d bit=%04x\n",
    s->errutim,
    D(s->errctim,26,0),
    D(s->errport,FT3D_NUMO-1,0),
    D(s->errport,31,16) );

  P("30-32 busy=%08x%s%s%s busy=%03x bsyin=%03x errin=%03x\n",
    s->busy,
    Bs(s->busy, 26, " BUSY"),
    Bs(s->busy, 25, " ERRIN"),
    Bs(s->busy, 24, " clkerr"),
    D(s->busy,   FT3D_NUMO-1, 0),
    D(s->obsyin, FT3D_NUMO-1, 0),
    D(s->oerrin, FT3D_NUMO-1, 0) );


  P("33-37 linkdn=%03x up=%03x alive=%03x b2ldn=%03x plldn=%03x\n",
    D(s->olinkdn, FT3D_NUMO-1, 0),
    D(s->olinkup, FT3D_NUMO-1, 0),
    D(s->oalive, FT3D_NUMO-1, 0),
    D(s->b2ldn, FT3D_NUMO-1, 0),
    D(s->plldn, FT3D_NUMO-1, 0));

  for (i=0; i<FT3D_NUMO; i++) {
    if (B(s->olinkup,i)) {
      P("%02x%02x%02x  O%d=%08x %08x %08x",
	0x3c+i, 0x46+i, 0x70+i, i, s->odata[i], s->odatb[i], s->odatc[i],
	D(s->odata[i],11,4) ? "" : Bs(s->odatb[i],31," busy"),
	Bs(s->odata[i],11," feeerr"),
	B(s->odata[i],1) ? Bs(s->odata[i],10," ttlost") : " ttdn",
	B(s->odata[i],3) ? Bs(s->odata[i],9," b2llost") : " b2ldn",
	Bs(s->odata[i],9," b2ldn"),
	Bs(s->odata[i],8," tagerr"),
	Ds(s->odata[i],7,6," fifoerr"),
	Ds(s->odata[i],5,4," seu"));
      if (D(s->odata[i],3,0) == 0x0f) {
	P(" tag=%d(%d) cnt=%d\n",
	  D(s->odatb[i],27,16), /* tag */
	  D(s->odatb[i],29,28), /* diff */
	  D(s->odatb[i],15,0)); /* cntb2l */
      } else {
	P("%s%s\n",
	  B(s->odata[i],2) ? "" : " plldn",
	  B(s->odata[i],2) ? "" : " not-alive");
      }
    }
  }
}
/* ---------------------------------------------------------------------- *\
   stat3d012
\* ---------------------------------------------------------------------- */
void
stat3d012(ftsw_t *ftsw, int ftswid)
{
  struct timeval tv;
  ft3d_t p;
  
  /* retrieve time just before reading registers */
  /* read registers */
  regs3d012(ftsw, &tv, &p, &p);

  statft_headline(ftswid, p.fpgaid, p.fpgaver, &tv);

  statft_assert(&tv, ftswid, p.conf, p.fpgaver, p.utime, p.jpll);
  
  if (verbose)  {
    /* stat3d_verbose(&tv, &p, &p); */
  } else {
    summary3d012(&tv, &p, &p);
  }
}
