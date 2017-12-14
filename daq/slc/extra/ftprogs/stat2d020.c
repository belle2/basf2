/*
  stat2d020.c
  
  20141226 for ft[23][dr]020 and on
  20150317 stata,b,c address fix
  20160106 jpll copied from ft2p026
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
#include "ft2d020.h"

typedef ft2d_t fast_t;
typedef ft2d_t slow_t;

#include "statft.h"

/* ---------------------------------------------------------------------- *\
   regs_ft2d020
\* ---------------------------------------------------------------------- */
void
regs_ft2d020(ftsw_t *ftsw, struct timeval *tvp, fast_t *f, slow_t *s)
{
  int i;

  gettimeofday(tvp, 0);

  s->ftswid     = read_ftsw(ftsw, FT2D_FTSWID);
  s->cpldver    = read_ftsw(ftsw, FT2D_CPLDVER);
  s->conf       = read_ftsw(ftsw, FT2D_CONF);
  s->clksel     = read_ftsw(ftsw, FT2D_CLKSEL);
  s->fpgaid     = read_ftsw(ftsw, FT2D_FPGAID);
  s->fpgaver    = read_ftsw(ftsw, FT2D_FPGAVER);
  f->utime      = read_ftsw(ftsw, FT2D_UTIME);
  f->ctime      = read_ftsw(ftsw, FT2D_CTIME);
  s->exprun     = read_ftsw(ftsw, FT2D_EXPRUN);
  s->dmask      = read_ftsw(ftsw, FT2D_DMASK);
  s->addr       = read_ftsw(ftsw, FT2D_ADDR);
  s->suser      = read_ftsw(ftsw, FT2D_SUSER);
  s->jpd        = read_ftsw(ftsw, FT2D_JPD);
  s->jpll       = read_ftsw(ftsw, FT2D_JPLL);
  f->ttup       = read_ftsw(ftsw, FT2D_TTUP);
  f->trgtag     = read_ftsw(ftsw, FT2D_TRGTAG);
  f->trgtyp     = read_ftsw(ftsw, FT2D_TRGTYP);
  s->errutim    = read_ftsw(ftsw, FT2D_ERRUTIM);
  s->errctim    = read_ftsw(ftsw, FT2D_ERRCTIM);
  f->errport    = read_ftsw(ftsw, FT2D_ERRPORT);
  f->errbit     = read_ftsw(ftsw, FT2D_ERRBIT);
  s->regdbg     = read_ftsw(ftsw, FT2D_REGDBG);
  f->busy       = read_ftsw(ftsw, FT2D_BUSY);
  f->obsyin     = read_ftsw(ftsw, FT2D_OBSYIN);
  f->oerrin     = read_ftsw(ftsw, FT2D_OERRIN);
  f->olinkdn    = read_ftsw(ftsw, FT2D_OLINKDN);
  f->oalive     = read_ftsw(ftsw, FT2D_OALIVE);
  f->olinkup    = read_ftsw(ftsw, FT2D_OLINKUP);
  f->b2ldn      = read_ftsw(ftsw, FT2D_B2LDN);
  f->plldn      = read_ftsw(ftsw, FT2D_PLLDN);
  for (i=0; i<=11; i++) {
    f->odata[i]   = read_ftsw(ftsw, FT2D_ODATA + i*4);
  };
  for (i=0; i<=11; i++) {
    f->odatb[i]   = read_ftsw(ftsw, FT2D_ODATB + i*4);
  };
  for (i=0; i<=11; i++) {
    f->odatc[i]   = read_ftsw(ftsw, FT2D_ODATC + i*4);
  };
  s->omanual    = read_ftsw(ftsw, FT2D_OMANUAL);
  s->clrdelay   = read_ftsw(ftsw, FT2D_CLRDELAY);
  s->incdelay   = read_ftsw(ftsw, FT2D_INCDELAY);
  s->bits       = read_ftsw(ftsw, FT2D_BITS);
  for (i=0; i<=7; i++) {
    s->dumpo[i]   = read_ftsw(ftsw, FT2D_DUMPO + i*4);
  };
  for (i=0; i<=9; i++) {
    s->dumpb[i]   = read_ftsw(ftsw, FT2D_DUMPB + i*4);
  };
  s->dumpk      = read_ftsw(ftsw, FT2D_DUMPK);
  s->dumpi      = read_ftsw(ftsw, FT2D_DUMPI);
  s->idump      = read_ftsw(ftsw, FT2D_IDUMP);
  s->vio        = read_ftsw(ftsw, FT2D_VIO);
  s->selila     = read_ftsw(ftsw, FT2D_SELILA);
}
/* ---------------------------------------------------------------------- *\
   summary2d020
\* ---------------------------------------------------------------------- */
void
summary2d020(struct timeval *tvp, fast_t *f, slow_t *s)
{
  int i;
  static const char *clksrc[] = { "in", "xtal", "fmc", "pin" };
  
  /* only useful registers */
  P("17   dmask=%08x\n",
    s->dmask);

  P("1f    jpll=%08x clk=%s%s%s%s\n",
    s->jpll,
    clksrc[D(s->jpll,29,28)],
    B(s->jpll,31) ? "" : " PLL-error",
    B(s->jpll,30) ? "" : " DCM-error",
    D(s->jpll,27,24) == 0x0c ? " GOOD-CLOCK" : " no/bad-clock");
    
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
    D(s->errport,FT2D_NUMO-1,0),
    D(s->errport,31,16) );

  P("30-32 busy=%08x%s%s%s busy=%03x bsyin=%03x errin=%03x\n",
    s->busy,
    Bs(s->busy, 26, " BUSY"),
    Bs(s->busy, 25, " ERRIN"),
    Bs(s->busy, 24, " clkerr"),
    D(s->busy,   FT2D_NUMO-1, 0),
    D(s->obsyin, FT2D_NUMO-1, 0),
    D(s->oerrin, FT2D_NUMO-1, 0) );


  P("33-37 linkdn=%03x up=%03x alive=%03x b2ldn=%03x plldn=%03x\n",
    D(s->olinkdn, FT2D_NUMO-1, 0),
    D(s->olinkup, FT2D_NUMO-1, 0),
    D(s->oalive, FT2D_NUMO-1, 0),
    D(s->b2ldn, FT2D_NUMO-1, 0),
    D(s->plldn, FT2D_NUMO-1, 0));

  for (i=0; i<FT2D_NUMO; i++) {
    if (B(s->olinkup,i)) {
      P("%02x%02x%02x  O%d=%08x %08x %08x",
        FTSWADDR(FT2D_ODATA)+i,
        FTSWADDR(FT2D_ODATB)+i,
        FTSWADDR(FT2D_ODATC)+i,
        i, s->odata[i], s->odatb[i], s->odatc[i],
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
   dump2d020
\* ---------------------------------------------------------------------- */
void
dump2d020(ftsw_t *ftsw)
{
  /* to make dump0..7,k working, it has to fetched once by writing 1 to 89 */
  slow_t sbuf;
  slow_t *s = &sbuf;
  uint32_t idump = read_ftsw(ftsw, FT2D_IDUMP);
  int i;
  
  if ((idump & 0x800) == 0) {
    write_ftsw(ftsw, FT2D_DUMPI, 1);
    usleep(100*1000); /* 100ms sleep */
  }
  for (i=0; i<=7; i++) {
    s->dumpo[i]   = read_ftsw(ftsw, FT2D_DUMPO + i*4);
  };
  for (i=0; i<=9; i++) {
    s->dumpb[i]   = read_ftsw(ftsw, FT2D_DUMPB + i*4);
  };
  s->dumpk      = read_ftsw(ftsw, FT2D_DUMPK);
  s->dumpi      = read_ftsw(ftsw, FT2D_DUMPI);

  statft_dump(s->dumpk, s->dumpi, s->dumpo, s->dumpb);
}
/* ---------------------------------------------------------------------- *\
   stat2d020
\* ---------------------------------------------------------------------- */
void
stat2d020(ftsw_t *ftsw, int ftswid)
{
  struct timeval tv;
  ft2d_t p;
  
  /* retrieve time just before reading registers */
  /* read registers */
  regs_ft2d020(ftsw, &tv, &p, &p);

  statft_assert(&tv, ftswid, p.conf, p.fpgaver, p.utime, p.jpll);
  
  if (showdump) {
    dump2d020(ftsw);
    return;
  }
  
  statft_headline(ftswid, p.fpgaid, p.fpgaver, &tv);

  if (verbose)  {
    /* stat2d_verbose(&tv, &p, &p); */
  } else {
    summary2d020(&tv, &p, &p);
  }
}
