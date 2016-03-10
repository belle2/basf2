/*

  20140822 fix 3b message
  20141029 fix not-alive, b2llost
  20141216 renamed to stat2p002, applicable up to ft2p013
  
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
#include "ft2p002.h"

typedef ft2p_t fast_t;
typedef ft2p_t slow_t;

#include "statft.h"

/* ---------------------------------------------------------------------- *\
   regs2p002
\* ---------------------------------------------------------------------- */
void
regs2p002(ftsw_t *ftsw, struct timeval *tvp, fast_t *f, slow_t *s)
{
  int i;

  gettimeofday(tvp, 0);

  s->ftswid     = read_ftsw(ftsw, FT2P_FTSWID);
  s->cpldver    = read_ftsw(ftsw, FT2P_CPLDVER);
  s->conf       = read_ftsw(ftsw, FT2P_CONF);
  s->clksel     = read_ftsw(ftsw, FT2P_CLKSEL);
  s->fpgaid     = read_ftsw(ftsw, FT2P_FPGAID);
  s->fpgaver    = read_ftsw(ftsw, FT2P_FPGAVER);
  s->setutim    = read_ftsw(ftsw, FT2P_SETUTIM);
  s->clkfreq    = read_ftsw(ftsw, FT2P_CLKFREQ);
  f->utime      = read_ftsw(ftsw, FT2P_UTIME);
  f->ctime      = read_ftsw(ftsw, FT2P_CTIME);
  s->exprun     = read_ftsw(ftsw, FT2P_EXPRUN);
  s->omask      = read_ftsw(ftsw, FT2P_OMASK);
  s->reg3s      = read_ftsw(ftsw, FT2P_REG3S);
  s->jtag       = read_ftsw(ftsw, FT2P_JTAG);
  s->tdo        = read_ftsw(ftsw, FT2P_TDO);
  s->jctl       = read_ftsw(ftsw, FT2P_JCTL);
  s->jreg       = read_ftsw(ftsw, FT2P_JREG);
  s->jpd        = read_ftsw(ftsw, FT2P_JPD);
  s->jpll       = read_ftsw(ftsw, FT2P_JPLL);
  s->reset      = read_ftsw(ftsw, FT2P_RESET);
  s->rstutim    = read_ftsw(ftsw, FT2P_RSTUTIM);
  s->rstctim    = read_ftsw(ftsw, FT2P_RSTCTIM);
  s->errutim    = read_ftsw(ftsw, FT2P_ERRUTIM);
  s->errctim    = read_ftsw(ftsw, FT2P_ERRCTIM);
  f->errport    = read_ftsw(ftsw, FT2P_ERRPORT);
  s->seltrg     = read_ftsw(ftsw, FT2P_SELTRG);
  s->tlimit     = read_ftsw(ftsw, FT2P_TLIMIT);
  f->tincnt     = read_ftsw(ftsw, FT2P_TINCNT);
  f->toutcnt    = read_ftsw(ftsw, FT2P_TOUTCNT);
  f->tlast      = read_ftsw(ftsw, FT2P_TLAST);
  f->stafifo    = read_ftsw(ftsw, FT2P_STAFIFO);
  s->enstat     = read_ftsw(ftsw, FT2P_ENSTAT);
  s->revopos    = read_ftsw(ftsw, FT2P_REVOPOS);
  s->revoin     = read_ftsw(ftsw, FT2P_REVOIN);
  s->badrevo    = read_ftsw(ftsw, FT2P_BADREVO);
  s->addr       = read_ftsw(ftsw, FT2P_ADDR);
  s->cmd        = read_ftsw(ftsw, FT2P_CMD);
  s->lckfreq    = read_ftsw(ftsw, FT2P_LCKFREQ);
  f->busy       = read_ftsw(ftsw, FT2P_BUSY);
  f->olinkup    = read_ftsw(ftsw, FT2P_OLINKUP);
  f->b2lup      = read_ftsw(ftsw, FT2P_B2LUP);
  f->err        = read_ftsw(ftsw, FT2P_ERR);
  for (i=0; i<=12; i++) {
    f->odata[i]   = read_ftsw(ftsw, FT2P_ODATA + i*4);
  };
  for (i=0; i<=12; i++) {
    f->odatb[i]   = read_ftsw(ftsw, FT2P_ODATB + i*4);
  };
  s->latency    = read_ftsw(ftsw, FT2P_LATENCY);
  f->udead      = read_ftsw(ftsw, FT2P_UDEAD);
  f->cdead      = read_ftsw(ftsw, FT2P_CDEAD);
  f->pdead      = read_ftsw(ftsw, FT2P_PDEAD);
  f->edead      = read_ftsw(ftsw, FT2P_EDEAD);
  f->fdead      = read_ftsw(ftsw, FT2P_FDEAD);
  f->rdead      = read_ftsw(ftsw, FT2P_RDEAD);
  for (i=0; i<=12; i++) {
    f->odead[i]   = read_ftsw(ftsw, FT2P_ODEAD + i*4);
  };
  for (i=0; i<=12; i++) {
    f->odatc[i]   = read_ftsw(ftsw, FT2P_ODATC + i*4);
  };
  s->omanual    = read_ftsw(ftsw, FT2P_OMANUAL);
  s->incdelay   = read_ftsw(ftsw, FT2P_INCDELAY);
  for (i=0; i<=7; i++) {
    s->dump[i]    = read_ftsw(ftsw, FT2P_DUMP + i*4);
  };
  s->dumpk      = read_ftsw(ftsw, FT2P_DUMPK);
  s->dumpi      = read_ftsw(ftsw, FT2P_DUMPI);
  s->oackq      = read_ftsw(ftsw, FT2P_OACKQ);
  for (i=0; i<=9; i++) {
    s->dumpb[i]   = read_ftsw(ftsw, FT2P_DUMPB + i*4);
  };
  s->disp       = read_ftsw(ftsw, FT2P_DISP);
  f->cntrst     = read_ftsw(ftsw, FT2P_CNTRST);
  s->tdelay     = read_ftsw(ftsw, FT2P_TDELAY);
}
/* ---------------------------------------------------------------------- *\
   summary2p002
\* ---------------------------------------------------------------------- */
void
summary2p002(struct timeval *tvp, fast_t *f, slow_t *s)
{
  int clkfreq;
  double tdiff;
  int i;
  
  /* a few things to be calculated beforehand */
  clkfreq = D(s->clkfreq,23,0) | 0x07000000;
  tdiff = tvp->tv_sec + tvp->tv_usec * 1.0e-6
    - f->utime - (double)f->ctime / clkfreq;

  /* only useful registers */
  P("16  exprun=%08x exp %d run %d sub %d\n", s->exprun,
    D(s->exprun,31,22), D(s->exprun,21,8), D(s->exprun,7,0));

  P("18   omask=%08x s3q=%d clk=%02x o=%04x\n",
    s->omask,
    B(s->omask,31),
    D(s->omask,20,16),
    D(s->omask,12,0));

  P("20   reset=%08x%s%s", s->reset,
    B(s->reset,31) ? " no-FIFO" : "",
    B(s->reset,30) ? " auto-reset" : "");
  if (B(s->reset,24)) PP(" use-TLU(delay=%d)", D(s->reset,26,25));
  PP("%s%s%s%s%s%s\n",
     B(s->reset,16) ? " regbusy" : "",
     B(s->reset,2)  ? " BOR" : "",
     B(s->reset,23) ? " EB" : "",
     B(s->reset,21) ? " PAUSED" : "",
     B(s->reset,20) ? " RUNNING" : "",
     ! B(s->reset,20) || s->tlast ? "" : "(LIMIT)");	  

  P("28  seltrg=%08x %s\n", s->seltrg, dumtrgstr(s->seltrg));
  
  P("2a/2b trig=%08x %08x (in:%d -> out:%d)\n",
    s->tincnt, s->toutcnt, s->tincnt, s->toutcnt);
  
  P("29/2c last=%08x %08x (limit:%d <-> last:%d)\n",
    s->tlimit, s->tlast, s->tlimit, s->tlast);
  
  P("2d stafifo=%08x%s%s%s%s%s%s\n", s->stafifo,
    D(s->stafifo,31,28) ? "" : " some data",
    B(s->stafifo,31)    ? " full" : "",
    B(s->stafifo,30)    ? " orun" : "",
    B(s->stafifo,29)    ? " ahi" : "",
    B(s->stafifo,28)    ? " empty" : "",
    B(s->stafifo,0)     ? " trg-enabled" : " trg-DISABLED");

  P("38    busy=%08x%s%s%s%s O(%02x/%02x)\n", s->busy,
    B(s->busy,29) ? " BUSY" : "",
    B(s->busy,13) ? " bsyin" : "",
    B(s->busy,30) ? " pipeful" : "",
    B(s->busy,31) ? " fifoerr" : "",
    D(s->busy,28,16), /* obusy */
    D(s->busy,12,0));  /* obsysig */

  P("39 olinkup=%08x tshort=%d clkerr=%d alive O(%04x) linkup O(%04x)\n",
    s->olinkup,
    B(s->olinkup,31),
    B(s->olinkup,30),
    D(s->olinkup,28,16),
    D(s->olinkup,12,0));

  P("3a   b2lup=%08x b2l(%02x/%02x) pll(%02x/%02x)\n",
    s->b2lup,
    D(s->b2lup,15,8),
    D(s->b2lup,31,24),
    D(s->b2lup,7,0),
    D(s->b2lup,23,16));

  P("3b     err=%08x%s%s%s%s ictrl=%d oerr=%04x odn=%04x\n",
    f->err,
    B(f->err, 31) ? " ERR" : "",
    B(f->err, 31) ? (B(f->err, 30) ? "IN" : "OR") : "",
    B(f->err, 29) ? " b2l" : "",
    B(f->err, 13) ? " linerr" : "",
    D(f->err, 15, 14),
    D(f->err, 28, 16),
    D(f->err, 12, 0) );
  
  P("25 errport=%08x", f->errport);
  if (D(f->errport,12,0)) P(" src=%03x", D(f->errport,12,0));
  P("%s", Ds(f->errport,17,16," seu"));
  P("%s", Ds(f->errport,19,18," fifo"));
  P("%s", Bs(f->errport,20," tagerr"));
  P("%s", Bs(f->errport,21," b2llost"));
  P("%s", Bs(f->errport,22," ttlost"));
  P("%s", Bs(f->errport,23," feeerr"));
  P("%s", Ds(f->errport,27,24," HSLB-"));
  P("%s", Bs(f->errport,24," A"));
  P("%s", Bs(f->errport,25," B"));
  P("%s", Bs(f->errport,26," C"));
  P("%s", Bs(f->errport,27," D"));
  P("\n");

  for (i=0; i<13; i++) {
    if (B(s->olinkup,i)) {
      P("%02x%02x%02x  O%d=%08x %08x %08x%s%s%s%s%s%s",
	0x3c+i, 0x4a+i, 0x70+i, i, s->odata[i], s->odatb[i], s->odatc[i],
	D(s->odata[i],11,4) ? "" : Bs(s->odatb[i],31," busy"),
	Bs(s->odata[i],11," feeerr"),
	B(s->odata[i],1) ? Bs(s->odata[i],10," ttlost") : " ttdn",
	B(s->odata[i],3) ? Bs(s->odata[i],9," b2llost") : " b2ldn",
	Bs(s->odata[i],8," tagerr"),
	Ds(s->odata[i],7,6," fifoerr"),
	Ds(s->odata[i],5,4," seu"));
      if (D(s->odata[i],3,0) == 0x0f) {
	P(" tag=%d(%d) cnt=%d d=%5.3fs\n",
	  D(s->odatb[i],27,16), /* tag */
	  D(s->odatb[i],29,28), /* diff */
	  D(s->odatb[i],15,0), /* cntb2l */
	  D(s->odead[i],31,16) + (double)D(s->odead[i],15,0)/(clkfreq>>11));
      } else {
	P("%s%s\n",
	  B(s->odata[i],2) ? "" : " plldn",
	  B(s->odata[i],0) ? "" : " not-alive");
      }
    }
  }
  
  P("59 latency=%08x maxtrig=%d maxtime=%4.2fus\n", s->latency,
    D(s->latency,31,24), D(s->latency,23,0) * 7.8e-3);

  /* -- 5a..6f -- */
  P("5a/5b dead=%08x %08x %1d.%1.0fs\n", f->udead, f->cdead,
    f->udead, (double)D(f->cdead,26,0) / clkfreq * 10);

  P("5c-5f PEFR=%08x %08x %08x %08x %3.1fs %3.1fs %3.1fs %3.1fs\n",
    s->pdead, s->edead, s->fdead, s->rdead,
    D(s->pdead,31,16) + (double)D(s->pdead,15,0)/(clkfreq>>11),
    D(s->edead,31,16) + (double)D(s->edead,15,0)/(clkfreq>>11),
    D(s->fdead,31,16) + (double)D(s->fdead,15,0)/(clkfreq>>11),
    D(s->rdead,31,16) + (double)D(s->rdead,15,0)/(clkfreq>>11));

  g_ftstat.exp = D(s->exprun,31,22);
  g_ftstat.run = D(s->exprun,21,8);
  g_ftstat.sub = D(s->exprun,7,0);
  g_ftstat.tincnt = f->tincnt;
  g_ftstat.rateall = 0;//rateall;
  g_ftstat.atrigc = 0;//f->atrigc;
  g_ftstat.raterun = 0;//raterun;
  g_ftstat.toutcnt = 0;//f->toutcnt;
  g_ftstat.rateout = 0;//rateout;
  g_ftstat.tlimit = s->tlimit;
  g_ftstat.tlast = f->tlast;
  sprintf(g_ftstat.reset, "%s%s%s%s%s%s%s%s",
	  B(s->reset,31) ? " no-FIFO" : "",
	  B(s->reset,30) ? " auto-reset" : "",
	  B(s->reset,16) ? " regbusy" : "",
	  B(s->reset,2)  ? " BOR" : "",
	  B(s->reset,23) ? " EB" : "",
	  B(s->reset,21) ? " PAUSED" : "",
	  B(s->reset,20) ? " RUNNING" : "",
	  ! B(s->reset,20) || s->tlast ? "" : "(LIMIT)");
  sprintf(g_ftstat.stafifo, "%s%s%s%s%s%s",
	  D(s->stafifo,31,28) ? "" : " some data",
	  B(s->stafifo,31)    ? " full" : "",
	  B(s->stafifo,30)    ? " orun" : "",
	  B(s->stafifo,29)    ? " ahi" : "",
	  B(s->stafifo,28)    ? " empty" : "",
	  B(s->stafifo,0)     ? " trg-enabled" : " trg-DISABLED");
  sprintf(g_ftstat.busy, "%s%s%s%s",
	  B(s->busy,29) ? " BUSY" : "",
	  B(s->busy,13) ? " bsyin" : "",
	  B(s->busy,30) ? " pipeful" : "",
	  B(s->busy,31) ? " fifoerr" : "");
  sprintf(g_ftstat.err, "%s%s%s%s",
	  B(f->err, 31) ? " ERR" : "",
	  B(f->err, 31) ? (B(f->err, 30) ? "IN" : "OR") : "",
	  B(f->err, 29) ? " b2l" : "",
	  B(f->err, 13) ? " linerr" : "");
  g_ftstat.errportsrc = (D(f->errport,12,0))? D(f->errport,12,0): 0;
  sprintf(g_ftstat.errport, "%s%s%s%s%s%s%s%s%s%s%s",
	  Ds(f->errport,17,16," seu"),
	  Ds(f->errport,19,18," fifo"),
	  Bs(f->errport,20," tagerr"),
	  Bs(f->errport,21," b2llost"),
	  Bs(f->errport,22," ttlost"),
	  Bs(f->errport,23," feeerr"),
	  Ds(f->errport,27,24," HSLB-"),
	  Bs(f->errport,24," A"),
	  Bs(f->errport,25," B"),
	  Bs(f->errport,26," C"),
	  Bs(f->errport,27," D"));
}
/* ---------------------------------------------------------------------- *\
   color2p002
\* ---------------------------------------------------------------------- */
color2p002(ft2p_t *p)
{
  int i;
  static int toutcnt_sav = -1;
  static int tincnt_sav = -1;
  static int first = 1;
  char buf[80];
  void setcolor(int color, const char *text);
  time_t t0 = (time_t)p->rstutim;
  time_t t1 = (time_t)p->errutim;
  int dt = p->utime - p->rstutim;
  struct tm *tp;
  char trst[80];
  char terr[80];
  static int tstart = 0;
  tp = localtime(&t0);
  sprintf(trst, "%04d.%02d.%02d %02d:%02d:%02d",
	  tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday,
	  tp->tm_hour, tp->tm_min, tp->tm_sec);
  tp = localtime(&t1);
  sprintf(terr, "%04d.%02d.%02d %02d:%02d:%02d",
	  tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday,
	  tp->tm_hour, tp->tm_min, tp->tm_sec);
  if (first) {
    setcolor(7, "starting statft..."); /* white */
  } else if (! B(p->stafifo, 0) && B(p->reset, 21)) { /* paused */
    setcolor(2, "PAUSED"); /* green */
  } else if (B(p->reset, 20) && p->tlast == 0) { /* limit */
    setcolor(2, "at trigger number limit"); /* green */
  } else if (p->errport) { /* ERROR */
    if (tstart == p->rstutim) {
      sprintf(buf, "ERROR (at %s run for %ds)", terr, (int)(t1 - t0));
    } else {
      sprintf(buf, "ERROR (at %s while not runnning)", terr);
    }
    setcolor(5, buf); /* magenta */
  } else if (! B(p->stafifo, 0)) { /* trgstop */
    if (tstart) {
      sprintf(buf, "READY (run is stopped at %s after %ds)",
	      trst, p->rstutim - tstart);
    } else {
      sprintf(buf, "READY (run is stopped at %s)", trst);
    }
    setcolor(3, buf); /* yellow */
  } else if ((p->seltrg & 7) == 0) { /* none trigger */
    setcolor(3, "READY (trigger is disabled)"); /* yellow */
  } else if (B(p->busy, 10)) { /* busy */
    setcolor(1, "BUSY");
  } else if (p->toutcnt == toutcnt_sav &&
	     B(p->reset,23) && B(p->reset,16)) { /* EB */
    setcolor(1, "BUSY from EB");
  } else if (p->toutcnt == toutcnt_sav &&
	     ! B(p->reset,23) && B(p->reset,16)) { /* EB */
    setcolor(1, "EB0 is gone");
  } else if (p->tincnt == tincnt_sav) { /* no input */
    setcolor(2, "no in-coming trigger"); /* green */
  } else if (p->toutcnt == toutcnt_sav) { /* not-updated */
    setcolor(1, "BUSY? tout is not updated");
  } else {
    if (tstart != p->rstutim) tstart = p->rstutim;
    sprintf(buf, "RUNNING (about %3.1fHz since %s for %ds)",
	    (double)(p->toutcnt - toutcnt_sav) / interval, trst, dt);
    setcolor(6, buf);
  }
  toutcnt_sav = p->toutcnt;
  tincnt_sav = p->tincnt;
  first = 0;
}
/* ---------------------------------------------------------------------- *\
   stat2p002
\* ---------------------------------------------------------------------- */
void
stat2p002(ftsw_t *ftsw, int ftswid, int showcolor)
{
  struct timeval tv;
  ft2p_t p;
  static char *dest[] = { "OPT-O13", "OPT-O14", "OPT-O15", "OPT-O16",
			  "OPT-O17", "OPT-O18", "OPT-O19", "OPT-O20",
			  "TTRX-O3", "TTRX-O5", "TTRX-O7", "TTRX-O9",
			  "TTRX-O11" };

  /* retrieve time just before reading registers */
  /* read registers */
  regs2p002(ftsw, &tv, &p, &p);

  if (showcolor) color2p002(&p);

  statft_assert(&tv, ftswid, p.conf, p.fpgaver, p.utime, p.jpll);
  
  statft_headline(ftswid, p.fpgaid, p.fpgaver, &tv);

  if (verbose)  {
    /* stat2p_verbose(&tv, &p, &p); */
  } else {
    summary2p002(&tv, &p, &p);
  }
}
