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
#include "ft3d006.h"

typedef ft3d_t fast_t;
typedef ft3d_t slow_t;

#include "statft.h"

/* ---------------------------------------------------------------------- *\
   regs3d006
\* ---------------------------------------------------------------------- */
void
regs3d006(ftsw_t *ftsw, struct timeval *tvp, fast_t *f, slow_t *s)
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
  f->olinkup    = read_ftsw(ftsw, FT3D_OLINKUP);
  f->b2ldn      = read_ftsw(ftsw, FT3D_B2LDN);
  f->b2lup      = read_ftsw(ftsw, FT3D_B2LUP);
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
    s->dump[i]    = read_ftsw(ftsw, FT3D_DUMP + i*4);
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
   summary3d006
\* ---------------------------------------------------------------------- */
void
summary3d006(struct timeval *tvp, fast_t *f, slow_t *s)
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
    D(s->errport,9,0),
    D(s->errport,31,16) );

  P("38    busy=%08x%s%s%s%s%s busy=%03x(%03x)\n",
    s->busy,
    Bs(s->busy, 26, " BSYIN"),
    Bs(s->busy, 25, " ERRIN"),
    Bs(s->busy, 24, " clkerr"),
    Bs(s->busy, 22, " linerr"),
    B(s->busy, 21) ? "" : " b2ldn",
    D(s->busy, 19, 10),
    D(s->busy, 9, 0) );

  P("39  linkup=%08x up=%03x alive=%03x dn=%03x\n",
    s->olinkup,
    D(s->olinkup, 9, 0),
    D(s->olinkup, 19, 10),
    D(s->olinkup, 29, 20) );

  P("3a   b2ldn=%08x b2l=%03x pll=%03x errin=%03x\n",
    s->b2ldn,
    D(s->b2ldn,29,20),
    D(s->b2ldn,19,10),
    D(s->b2ldn,9,0));

  P("3b   b2lup=%08x b2l=%03x pll=%03x ictrl=%d\n",
    s->b2lup,
    D(s->b2lup,19,10),
    D(s->b2lup,9,0),
    D(s->b2lup,31,30));
    
  for (i=0; i<9; i++) {
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
   stat3d006
\* ---------------------------------------------------------------------- */
void
stat3d006(ftsw_t *ftsw, int ftswid)
{
  struct timeval tv;
  ft3d_t p;
  
  /* retrieve time just before reading registers */
  /* read registers */
  regs3d006(ftsw, &tv, &p, &p);

  statft_assert(&tv, ftswid, p.conf, p.fpgaver, p.utime, p.jpll);
  
  statft_headline(ftswid, p.fpgaid, p.fpgaver, &tv);

  if (verbose)  {
    /* stat3d_verbose(&tv, &p, &p); */
  } else {
    summary3d006(&tv, &p, &p);
  }
}
