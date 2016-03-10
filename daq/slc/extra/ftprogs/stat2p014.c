/*

  20141216 new for ft2p014 and on
  20141218 daedtime update
  20150108 ictrl definition swap
  20150306 statft_jitter
  20150318 cntrst -> errsrc
  20150327 ERROR and READY run start time printing
  20150529 errsrc
  20151209 GLOBAL/LOCAL at omask line
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
#include "ft2p014.h"

#if !defined(TTMASTER) && !defined(POCKET_TTD)
typedef ft2p_t fast_t;
typedef ft2p_t slow_t;
#endif

#include "statft.h"

/* ---------------------------------------------------------------------- *\
   regs2p014
\* ---------------------------------------------------------------------- */
void
regs2p014(ftsw_t *ftsw, struct timeval *tvp, fast_t *f, slow_t *s)
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
  s->addr       = read_ftsw(ftsw, FT2P_ADDR);
  s->cmd        = read_ftsw(ftsw, FT2P_CMD);
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
  f->obusy      = read_ftsw(ftsw, FT2P_OBUSY);
  f->obsyin     = read_ftsw(ftsw, FT2P_OBSYIN);
  f->oerrin     = read_ftsw(ftsw, FT2P_OERRIN);
  f->olinkdn    = read_ftsw(ftsw, FT2P_OLINKDN);
  f->oalive     = read_ftsw(ftsw, FT2P_OALIVE);
  f->olinkup    = read_ftsw(ftsw, FT2P_OLINKUP);
  f->b2ldn      = read_ftsw(ftsw, FT2P_B2LDN);
  f->plldn      = read_ftsw(ftsw, FT2P_PLLDN);
  for (i=0; i<=11; i++) {
    f->odata[i]   = read_ftsw(ftsw, FT2P_ODATA + i*4);
  };
  for (i=0; i<=11; i++) {
    f->odatb[i]   = read_ftsw(ftsw, FT2P_ODATB + i*4);
  };
  for (i=0; i<=11; i++) {
    f->odatc[i]   = read_ftsw(ftsw, FT2P_ODATC + i*4);
  };
  s->omanual    = read_ftsw(ftsw, FT2P_OMANUAL);
  s->clrdelay   = read_ftsw(ftsw, FT2P_CLRDELAY);
  s->incdelay   = read_ftsw(ftsw, FT2P_INCDELAY);
  s->latency    = read_ftsw(ftsw, FT2P_LATENCY);
  s->lckfreq    = read_ftsw(ftsw, FT2P_LCKFREQ);
  s->revopos    = read_ftsw(ftsw, FT2P_REVOPOS);
  s->revocand   = read_ftsw(ftsw, FT2P_REVOCAND);
  s->badrevo    = read_ftsw(ftsw, FT2P_BADREVO);
#if !defined(TTMASTER) && !defined(POCKET_TTD)
  s->oackq      = read_ftsw(ftsw, FT2P_OACKQ);
  for (i=0; i<=2; i++) {
    s->dbg[i]     = read_ftsw(ftsw, FT2P_DBG + i*4);
  };
  s->disp       = read_ftsw(ftsw, FT2P_DISP);
#endif
  f->errsrc     = read_ftsw(ftsw, FT2P_ERRSRC);
  s->tdelay     = read_ftsw(ftsw, FT2P_TDELAY);
  f->atimec     = read_ftsw(ftsw, FT2P_ATIMEC);
  f->atrigc     = read_ftsw(ftsw, FT2P_ATRIGC);
  f->abusyc     = read_ftsw(ftsw, FT2P_ABUSYC);
  f->tbusyc     = read_ftsw(ftsw, FT2P_TBUSYC);
  f->cbusyc     = read_ftsw(ftsw, FT2P_CBUSYC);
  f->pbusyc     = read_ftsw(ftsw, FT2P_PBUSYC);
  f->fbusyc     = read_ftsw(ftsw, FT2P_FBUSYC);
  f->rbusyc     = read_ftsw(ftsw, FT2P_RBUSYC);
  s->reg3s      = read_ftsw(ftsw, FT2P_REG3S);
  for (i=0; i<=11; i++) {
    f->obusyc[i] = read_ftsw(ftsw, FT2P_OBUSYC + i*4);
  };
}
/* ---------------------------------------------------------------------- *\
   dump2p014
\* ---------------------------------------------------------------------- */
#if !defined(TTMASTER) && !defined(POCKET_TTD)
void
dump2p014(ftsw_t *ftsw)
{
  /* to make dump0..7,k working, it has to fetched once by writing 1 to 89 */
  slow_t sbuf;
  slow_t *s = &sbuf;
  uint32_t idump = read_ftsw(ftsw, FT2P_DISP);
  int i;
  
  if ((idump & 0x800) == 0) {
    write_ftsw(ftsw, FT2P_DUMPI, 1);
    usleep(100*1000); /* 100ms sleep */
  }
  for (i=0; i<=7; i++) {
    s->dumpo[i]   = read_ftsw(ftsw, FT2P_DUMPO + i*4);
  };
  for (i=0; i<=9; i++) {
    s->dumpb[i]   = read_ftsw(ftsw, FT2P_DUMPB + i*4);
  };
  s->dumpk      = read_ftsw(ftsw, FT2P_DUMPK);
  s->dumpi      = read_ftsw(ftsw, FT2P_DUMPI);

  statft_dump(s->dumpk, s->dumpi, s->dumpo, s->dumpb);
}
#endif
/* ---------------------------------------------------------------------- *\
   summary2p014
\* ---------------------------------------------------------------------- */
void
summary2p014(struct timeval *tvp, fast_t *f, slow_t *s)
{
  int clkfreq;
  double tdiff, tall, trun;
  double rateall, raterun, rateout;
  int i;
  
  /* a few things to be calculated beforehand */
  clkfreq = D(s->clkfreq,23,0) | 0x07000000;
  tdiff = tvp->tv_sec + tvp->tv_usec * 1.0e-6
    - f->utime - (double)f->ctime / clkfreq;

  /* only useful registers */
  P("16  exprun=%08x exp %d run %d sub %d\n", s->exprun,
    D(s->exprun,31,22), D(s->exprun,21,8), D(s->exprun,7,0));
  
  P("17   omask=%08x s3q=%d clk=%02x o=%04x%s\n",
    s->omask,
    B(s->omask,31),
    D(s->omask,20,16),
    D(s->omask,12,0),
    B(s->omask,15) ? " LOCAL" : " GLOBAL");

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
     ! B(s->reset,20) || f->tlast ? "" : "(LIMIT)");

  P("28  seltrg=%08x %s\n", s->seltrg, dumtrgstr(s->seltrg));

  tall = (double)f->utime - (double)s->rstutim;
  trun = (double)s->errutim - (double)s->rstutim;
  if (trun <= 0) trun = tall;
  rateall = tall > 0 ? f->tincnt / tall : 0;
  raterun = trun > 0 ? f->atrigc / trun : 0;
  rateout = trun > 0 ? f->toutcnt / trun : 0;
  P("2a/2b trig %d(%3.1fHz) -> %d(%3.1fHz) -> %d(%3.1fHz)\n",
  /*P("2a/2b trig in:%d/%d -> out:%d(%3.1fHz)\n",*/
    f->tincnt, rateall,
    f->atrigc, raterun,
    f->toutcnt, rateout);
  
  P("29/2c limit %d <-> last %d\n",
    s->tlimit, f->tlast);
  
  P("2d stafifo=%08x%s%s%s%s%s%s\n", f->stafifo,
    D(f->stafifo,31,28) ? "" : " some data",
    B(f->stafifo,31)    ? " full" : "",
    B(f->stafifo,30)    ? " orun" : "",
    B(f->stafifo,29)    ? " ahi" : "",
    B(f->stafifo,28)    ? " empty" : "",
    B(f->stafifo,0)     ? " trg-enabled" : " trg-DISABLED");

  P("30-32 busy=%03x bsyin=%03x errin=%03x%s%s%s%s%s%s\n",
    D(f->obusy,11,0),
    D(f->obsyin,11,0),
    D(f->oerrin,11,0),
    B(f->obusy,31) ? " BUSY" : "",
    B(f->obusy,30) ? " bsyin" : "",
    B(f->obusy,29) ? " pipeline" : "",
    B(f->oerrin,31) ? " ERROR" : "",
    B(f->oerrin,30) ? " errin" : "",
    B(f->oerrin,29) ? " fifo" : "");

  P("25 errport=%08x", f->errport);
  if (D(f->errsrc,4,0)) {
    P(" port=%03x src=%02x", D(f->errport,11,0), D(f->errsrc,4,0));
    P("%s%s%s%s%s",
      Bs(f->errsrc,4," errin"),
      Bs(f->errsrc,3," b2ldn"),
      Bs(f->errsrc,2," clkerr"),
      Bs(f->errsrc,1," tshort"),
      Bs(f->errsrc,0," ttdn"));
  }
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

  P("33-35 b2tt-down=%03x/alive=%03x/up=%03x\n",
    D(f->olinkdn,11,0),
    D(f->oalive,11,0),
    D(f->olinkup,11,0));

  P("36-37 b2l-down=%03x pll-down=%03x%s%s%s%s\n",
    D(f->b2ldn,11,0),
    D(f->plldn,11,0),
    B(f->plldn,31) ? " clk-error" : "",
    B(f->plldn,30) ? " trgshort" : "",
    B(f->plldn,29) ? "" : " !ictrl",
    B(f->plldn,28) ? "" : " !dcm200");

  for (i=0; i<12; i++) {
    if (B(f->oalive,i)) {
      int fa = f->odata[i];
      int fb = f->odatb[i];
      int fc = f->odatc[i];
      P("%02x%02x%02x  %s%d%s%08x %08x %08x",
	FTSWADDR(FT2P_ODATA)+i,
        FTSWADDR(FT2P_ODATB)+i,
        FTSWADDR(FT2P_ODATC)+i,
        B(s->omask,i)?"o":"O", i, B(s->omask,i)?"!":"=",
        fa, fb, fc);

      P("%s%s%s%s%s%s%s",
	D(fa,11,4) ? "" : Bs(fb,31," busy"),
	Bs(fa,11," feeerr"),
	B(fa,1) ? Bs(fa,10," ttlost") : " ttdn",
	B(fa,3) ? Bs(fa,9," b2llost") : " b2ldn",
	B(fa,10) ? "" : Bs(fa,8," tagerr"),
	B(fa,10) ? "" : Ds(fa,7,6," fifoerr"),
	B(fa,10) ? "" : Ds(fa,5,4," seu"));
      if (D(fa,3,0) == 0x0f && ! B(fa,10)) {
	P(" tag=%d(%d) cnt=%d d=%4.2f%%\n",
	  D(fb,27,16), /* tag */
	  D(fb,29,28), /* diff */
	  D(fb,15,0), /* cntb2l */
          f->atimec ? f->obusyc[i] / (double)f->atimec * 100 : 0);
      } else {
	P("%s%s\n",
	  B(fa,2) ? "" : " plldn",
	  B(fa,0) ? "" : " not-alive");
      }
    }
  }
  
  P("7f latency=%08x maxtrig=%d maxtime=%4.2fus\n", s->latency,
    D(s->latency,31,24), D(s->latency,23,0) * 7.8e-3);

  /* -- 5a..6f -- */
  P("a0-a7 dead %4.2f%% (t=%4.2f%% c=%4.2f%% p=%4.2f%% f=%4.2f%% r=%4.2f%%)\n",
    f->atimec ? f->abusyc / (double)f->atimec * 100 : 0,
    f->atimec ? f->tbusyc / (double)f->atimec * 100 : 0,
    f->atimec ? f->cbusyc / (double)f->atimec * 100 : 0,
    f->atimec ? f->pbusyc / (double)f->atimec * 100 : 0,
    f->atimec ? f->fbusyc / (double)f->atimec * 100 : 0,
    f->atimec ? f->rbusyc / (double)f->atimec * 100 : 0);

  g_ftstat.exp = D(s->exprun,31,22);
  g_ftstat.run = D(s->exprun,21,8);
  g_ftstat.sub = D(s->exprun,7,0);
  g_ftstat.tincnt = f->tincnt;
  g_ftstat.rateall = rateall;
  g_ftstat.atrigc = f->atrigc;
  g_ftstat.raterun = raterun;
  g_ftstat.toutcnt = f->toutcnt;
  g_ftstat.rateout = rateout;
  g_ftstat.tlimit = s->tlimit;
  g_ftstat.tlast = f->tlast;
  g_ftstat.reset[0] = 0;
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
  sprintf(g_ftstat.busy, "%s%s%s",
	  B(f->obusy,31) ? " BUSY" : "",
	  B(f->obusy,30) ? " bsyin" : "",
	  B(f->obusy,29) ? " pipeline" : "");
  sprintf(g_ftstat.err, "%s%s%s",
	  B(f->oerrin,31) ? " ERROR" : "",
	  B(f->oerrin,30) ? " errin" : "",
	  B(f->oerrin,29) ? " fifo" : "");
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
   color2p014
\* ---------------------------------------------------------------------- */
#if !defined(TTMASTER) && !defined(POCKET_TTD)
color2p014(ft2p_t *p)
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
  } else if (D(p->errport,27,0) || D(p->errsrc,4,0)) { /* ERROR */
    if (tstart == p->rstutim) {
      sprintf(buf, "ERROR (%s -- %s for %ds)", trst, terr, (int)(t1 - t0));
    } else {
      sprintf(buf, "ERROR (at %s while not runnning)", terr);
    }
    setcolor(5, buf); /* magenta */
  } else if (! B(p->stafifo, 0)) { /* trgstop */
    if (t1) {
      sprintf(buf, "READY (%s -- %s for %ds)", trst, terr, (int)(t1 - t0));
    } else if (t0) {
      sprintf(buf, "READY (reset at %s)", trst);
    } else {
      sprintf(buf, "READY");
    }
    setcolor(3, buf); /* yellow */
  } else if ((p->seltrg & 7) == 0) { /* none trigger */
    setcolor(3, "READY (trigger is disabled)"); /* yellow */
  } else if (p->toutcnt == toutcnt_sav && B(p->obusy, 31)) { /* busy */
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
#endif
/* ---------------------------------------------------------------------- *\
   stat2p014
\* ---------------------------------------------------------------------- */
#if !defined(TTMASTER) && !defined(POCKET_TTD)
void
stat2p014(ftsw_t *ftsw, int ftswid, int showcolor)
{
  struct timeval tv;
  ft2p_t p;
  static char *dest[] = { "OPT-O13", "OPT-O14", "OPT-O15", "OPT-O16",
			  "OPT-O17", "OPT-O18", "OPT-O19", "OPT-O20",
			  "TTRX-O3", "TTRX-O5", "TTRX-O7", "TTRX-O9",
			  "TTRX-O11" };

  /* retrieve time just before reading registers */
  /* read registers */
  regs2p014(ftsw, &tv, &p, &p);

  if (showcolor) color2p014(&p);

  if (showdump) {
    dump2p014(ftsw);
    return;
  }
  
  statft_headline(ftswid, p.fpgaid, p.fpgaver, &tv);

  statft_assert(&tv, ftswid, p.conf, p.fpgaver, p.utime, p.jpll);
  
  if (verbose)  {
    /* stat2p_verbose(&tv, &p, &p); */
  } else {
    summary2p014(&tv, &p, &p);
  }
  if (showjitter) statft_jitter(ftsw, FT2P_JCTL, FT2P_JREG);
}
#endif
