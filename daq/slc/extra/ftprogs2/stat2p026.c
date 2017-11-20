/*
  20151209 GLOBAL/LOCAL at omask line
  20160106 branched from stat2p014
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

#include "ft2p026.h"

typedef ft2p_t fast_t;
typedef ft2p_t slow_t;
#include "ftsw.h"

#include "statft.h"

/* ---------------------------------------------------------------------- *\
   summary2p026
\* ---------------------------------------------------------------------- */
void summary2p026(struct timeval *tvp, fast_t *f, slow_t *s, char* ss)
{
  int clkfreq;
  double tdiff, tall, trun;
  double rateall, raterun, rateout;
  int i;
  static char *clksrc[] = { "in", "xtal", "fmc", "pin" };
  char css[100];

  /* a few things to be calculated beforehand */
  clkfreq = D(s->clkfreq,23,0) | 0x07000000;
  tdiff = tvp->tv_sec + tvp->tv_usec * 1.0e-6
    - f->utime - (double)f->ctime / clkfreq;

  /* only useful registers */
  sprintf(css, "16  exprun=%08x exp %d run %d sub %d\n", s->exprun,
	  D(s->exprun,31,22), D(s->exprun,21,8), D(s->exprun,7,0));
  strcat(ss, css);
  
  sprintf(css,  "17   omask=%08x s3q=%d clk=%02x o=%04x%s\n",
	  s->omask,
	  B(s->omask,31),
	  D(s->omask,20,16),
	  D(s->omask,12,0),
	  B(s->omask,15) ? " LOCAL" : " GLOBAL");
  strcat(ss, css);

  sprintf(css,  "1f    jpll=%08x clk=%s%s%s%s\n",
	  s->jpll,
	  clksrc[D(s->jpll,29,28)],
	  B(s->jpll,31) ? "" : " PLL-error",
	  B(s->jpll,30) ? "" : " DCM-error",
	  D(s->jpll,27,24) == 0x0c ? " GOOD-CLOCK" : " no/bad-clock");
  strcat(ss, css);

  sprintf(css,  "20   reset=%08x%s%s", s->reset,
	  B(s->reset,31) ? " no-FIFO" : "",
	  B(s->reset,30) ? " auto-reset" : "");
  if (B(s->reset,24)) sprintf(ss, " use-TLU(delay=%d)", D(s->reset,26,25));
  strcat(ss, css);
  sprintf(css,  "%s%s%s%s%s%s\n",
	  B(s->reset,16) ? " regbusy" : "",
	  B(s->reset,2)  ? " BOR" : "",
	  B(s->reset,23) ? " EB" : "",
	  B(s->reset,21) ? " PAUSED" : "",
	  B(s->reset,20) ? " RUNNING" : "",
	  ! B(s->reset,20) || f->tlast ? "" : "(LIMIT)");
  strcat(ss, css);

  sprintf(css,  "28  seltrg=%08x %s\n", s->seltrg, dumtrgstr(s->seltrg));

  tall = (double)f->utime - (double)s->rstutim;
  trun = (double)s->errutim - (double)s->rstutim;
  if (trun <= 0) trun = tall;
  rateall = tall > 0 ? f->tincnt / tall : 0;
  raterun = trun > 0 ? f->atrigc / trun : 0;
  rateout = trun > 0 ? f->toutcnt / trun : 0;
  sprintf(css,  "2a/2b trig %d(%3.1fHz) -> %d(%3.1fHz) -> %d(%3.1fHz)\n",
	  f->tincnt, rateall,
	  f->atrigc, raterun,
	  f->toutcnt, rateout);
  strcat(ss, css);

  sprintf(css,  "29/2c limit %d <-> last %d\n",
	  s->tlimit, f->tlast);
  strcat(ss, css);

  sprintf(css,  "2d stafifo=%08x%s%s%s%s%s%s\n", f->stafifo,
	  D(f->stafifo,31,28) ? "" : " some data",
	  B(f->stafifo,31)    ? " full" : "",
	  B(f->stafifo,30)    ? " orun" : "",
	  B(f->stafifo,29)    ? " ahi" : "",
	  B(f->stafifo,28)    ? " empty" : "",
    B(f->stafifo,0)     ? " trg-enabled" : " trg-DISABLED");
  strcat(ss, css);

  sprintf(css,  "30-32 busy=%03x bsyin=%03x errin=%03x%s%s%s%s%s%s\n",
	  D(f->obusy,11,0),
	  D(f->obsyin,11,0),
	  D(f->oerrin,11,0),
	  B(f->obusy,31) ? " BUSY" : "",
	  B(f->obusy,30) ? " bsyin" : "",
	  B(f->obusy,29) ? " pipeline" : "",
	  B(f->oerrin,31) ? " ERROR" : "",
	  B(f->oerrin,30) ? " errin" : "",
	  B(f->oerrin,29) ? " fifo" : "");
  strcat(ss, css);

  sprintf(css,  "25 errport=%08x", f->errport);
  strcat(ss, css);
  if (D(f->errsrc,4,0)) {
    sprintf(css,  " port=%03x src=%02x", D(f->errport,11,0), D(f->errsrc,4,0));
    strcat(ss, css);
    sprintf(css,  "%s%s%s%s%s",
	    Bs(f->errsrc,4," errin"),
	    Bs(f->errsrc,3," b2ldn"),
	    Bs(f->errsrc,2," clkerr"),
	    Bs(f->errsrc,1," tshort"),
	    Bs(f->errsrc,0," ttdn"));
    strcat(ss, css);
  }
  strcat(ss, css);
  sprintf(css,  "%s", Ds(f->errport,17,16," seu"));
  strcat(ss, css);
  sprintf(css,  "%s", Ds(f->errport,19,18," fifo"));
  strcat(ss, css);
  sprintf(css,  "%s", Bs(f->errport,20," tagerr"));
  strcat(ss, css);
  sprintf(css,  "%s", Bs(f->errport,21," b2llost"));
  strcat(ss, css);
  sprintf(css,  "%s", Bs(f->errport,22," ttlost"));
  strcat(ss, css);
  sprintf(css,  "%s", Bs(f->errport,23," feeerr"));
  strcat(ss, css);
  sprintf(css,  "%s", Ds(f->errport,27,24," HSLB-"));
  strcat(ss, css);
  sprintf(css,  "%s", Bs(f->errport,24," A"));
  strcat(ss, css);
  sprintf(css,  "%s", Bs(f->errport,25," B"));
  strcat(ss, css);
  sprintf(css,  "%s", Bs(f->errport,26," C"));
  strcat(ss, css);
  sprintf(css,  "%s", Bs(f->errport,27," D"));
  strcat(ss, css);
  sprintf(css,  "\n");
  strcat(ss, css);
  
  sprintf(css,  "33-35 b2tt-down=%03x/alive=%03x/up=%03x\n",
	  D(f->olinkdn,11,0),
	  D(f->oalive,11,0),
	  D(f->olinkup,11,0));
  strcat(ss, css);

  sprintf(css,  "36-37 b2l-down=%03x pll-down=%03x%s%s%s%s\n",
	  D(f->b2ldn,11,0),
	  D(f->plldn,11,0),
	  B(f->plldn,31) ? " clk-error" : "",
	  B(f->plldn,30) ? " trgshort" : "",
	  B(f->plldn,29) ? "" : " !ictrl",
	  B(f->plldn,28) ? "" : " !dcm200");
  strcat(ss, css);

  for (i=0; i<12; i++) {
    if (B(f->oalive,i)) {
      int fa = f->odata[i];
      int fb = f->odatb[i];
      int fc = f->odatc[i];
      sprintf(css,  "%02x%02x%02x  %s%d%s%08x %08x %08x",
	      FTSWADDR(FT2P_ODATA)+i,
	      FTSWADDR(FT2P_ODATB)+i,
	      FTSWADDR(FT2P_ODATC)+i,
	      B(s->omask,i)?"o":"O", i, B(s->omask,i)?"!":"=",
	      fa, fb, fc);
      strcat(ss, css);

      sprintf(css,  "%s%s%s%s%s%s%s",
	      D(fa,11,4) ? "" : Bs(fb,31," busy"),
	      Bs(fa,11," feeerr"),
	      B(fa,1) ? Bs(fa,10," ttlost") : " ttdn",
	      B(fa,3) ? Bs(fa,9," b2llost") : " b2ldn",
	      B(fa,10) ? "" : Bs(fa,8," tagerr"),
	      B(fa,10) ? "" : Ds(fa,7,6," fifoerr"),
	      B(fa,10) ? "" : Ds(fa,5,4," seu"));
      strcat(ss, css);
      if (D(fa,3,0) == 0x0f && ! B(fa,10)) {
	sprintf(css, " tag=%d(%d) cnt=%d d=%4.2f%%\n",
		D(fb,27,16), /* tag */
		D(fb,29,28), /* diff */
		D(fb,15,0), /* cntb2l */
		f->atimec ? f->obusyc[i] / (double)f->atimec * 100 : 0);
	strcat(ss, css);
      } else {
	sprintf(ss, "%s%s\n",
		B(fa,2) ? "" : " plldn",
		B(fa,0) ? "" : " not-alive");
	strcat(ss, css);
      }
    }
  }

  sprintf(css,  "7f latency=%08x maxtrig=%d maxtime=%4.2fus\n", s->latency,
	  D(s->latency,31,24), D(s->latency,23,0) * 7.8e-3);
  strcat(ss, css);
  
  /* -- 5a..6f -- */
  sprintf(css,  "a0-a7 dead %4.2f%% (t=%4.2f%% c=%4.2f%% p=%4.2f%% f=%4.2f%% r=%4.2f%%)\n",
	  f->atimec ? f->abusyc / (double)f->atimec * 100 : 0,
	  f->atimec ? f->tbusyc / (double)f->atimec * 100 : 0,
	  f->atimec ? f->cbusyc / (double)f->atimec * 100 : 0,
	  f->atimec ? f->pbusyc / (double)f->atimec * 100 : 0,
	  f->atimec ? f->fbusyc / (double)f->atimec * 100 : 0,
	  f->atimec ? f->rbusyc / (double)f->atimec * 100 : 0);
  strcat(ss, css);
}

/* ---------------------------------------------------------------------- * \
   color2p026
\* ---------------------------------------------------------------------- */
color2p026(fast_t *f, slow_t *s, char* ss, char* state)
{
  int i;
  static int toutcnt_sav = -1;
  static int tincnt_sav = -1;
  static int first = 1;
  char buf[80];
  void setcolor(int color, const char *text);
  time_t t0 = (time_t)s->rstutim;
  time_t t1 = (time_t)s->errutim;
  int dt = f->utime - s->rstutim;
  struct tm *tp;
  char trst[80];
  char terr[80];
  static int tstart = 0;
  double interval = 0;
  double unk = 0;
  int freq = s->clkfreq | 0x07000000;
  static uint32_t prev_u = 0;
  static uint32_t prev_c = 0;
  char css[100];
  
  if (prev_u && f->utime) {
    interval = ((f->utime - prev_u) * (double)freq +
                ((int32_t)f->ctime - (int32_t)prev_c)) / (double)freq;
    unk = interval;
    if (interval < 0.3 || interval > 10) interval = 0;
  }
  prev_u = f->utime;
  prev_c = f->ctime;

  tp = localtime(&t0);
  sprintf(trst, "%04d.%02d.%02d %02d:%02d:%02d",
	  tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday,
	  tp->tm_hour, tp->tm_min, tp->tm_sec);
  tp = localtime(&t1);
  sprintf(terr, "%04d.%02d.%02d %02d:%02d:%02d",
	  tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday,
	  tp->tm_hour, tp->tm_min, tp->tm_sec);
  if (first) {
    sprintf(css,  "starting statft...\n"); /* white */
    strcat(ss, css);
    sprintf(state, "STARTING");
  } else if (! B(f->stafifo, 0) && B(s->reset, 21)) { /* paused */
    sprintf(css,  "PAUSED\n"); /* green */
    strcat(ss, css);
    sprintf(state, "PAUSED");
  } else if (B(s->reset, 20) && f->tlast == 0) { /* limit */
    sprintf(css,  "at trigger number limit\n"); /* green */
    strcat(ss, css);
    sprintf(state, "PAUSED");
  } else if (D(f->errport,27,0) || D(s->errsrc,4,0)) { /* ERROR */
    if (tstart == s->rstutim) {
      sprintf(buf, "ERROR (%s -- %s for %ds)\n", trst, terr, (int)(t1 - t0));
    } else {
      sprintf(buf, "ERROR (at %s while not runnning)\n", terr);
    }
    sprintf(css, buf); /* magenta */
    strcat(ss, css);
    sprintf(state, "ERROR");
  } else if (! B(f->stafifo, 0)) { /* trgstop */
    if (t1) {
      sprintf(buf, "READY (%s -- %s for %ds)\n", trst, terr, (int)(t1 - t0));
    } else if (t0) {
      sprintf(buf, "READY (reset at %s)\n", trst);
    } else {
      sprintf(buf, "READY");
    }
    sprintf(css,  buf); /* yellow */
    strcat(ss, css);
    sprintf(state, "READY");
  } else if ((s->seltrg & 7) == 0) { /* none trigger */
    sprintf(css,  "READY (trigger is disabled)\n"); /* yellow */
    strcat(ss, css);
    sprintf(state, "READY");
  } else if (f->toutcnt == toutcnt_sav && B(f->obusy, 31)) { /* busy */
    sprintf(css,  "BUSY");
    strcat(ss, css);
    sprintf(state, "BUSY");
  } else if (f->toutcnt == toutcnt_sav &&
	     B(s->reset,23) && B(s->reset,16)) { /* EB */
    strcat(ss, css);
    sprintf(css,  "BUSY from EB\n");
    sprintf(state, "BUSY");
  } else if (f->toutcnt == toutcnt_sav &&
	     ! B(s->reset,23) && B(s->reset,16)) { /* EB */
    sprintf(css,  "EB0 is gone\n");
    strcat(ss, css);
    sprintf(state, "BUSY");
  } else if (f->tincnt == tincnt_sav) { /* no input */
    sprintf(buf, "RUNNING (no in-coming trigger, start at %s)\n", trst);
    sprintf(css,  buf); /* green */
    strcat(ss, css);
    sprintf(state, "RUNNING");
  } else if (f->toutcnt == toutcnt_sav) { /* not-updated */
    sprintf(css,  "BUSY? tout is not updated\n");
    strcat(ss, css);
    sprintf(state, "BUSY");
  } else {
    if (tstart != s->rstutim) tstart = s->rstutim;
    if (interval == 0) {
      sprintf(buf, "RUNNING (unknown freq %f since %s for %ds)\n",
              unk, trst, dt);
    } else {
      sprintf(buf, "RUNNING (about %3.1fHz since %s for %ds)",
              (double)(f->toutcnt - toutcnt_sav) / interval, trst, dt);
    }
    sprintf(css,  buf);
    strcat(ss, css);
    sprintf(state, "RUNNING");
  }
  toutcnt_sav = f->toutcnt;
  tincnt_sav = f->tincnt;
  first = 0;
}

/* ---------------------------------------------------------------------- *\
   regs2p026
\* ---------------------------------------------------------------------- */
void regs2p026(ftsw_t *ftsw, struct timeval *tvp, fast_t *f, slow_t *s)
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
   stat2p026
\* ---------------------------------------------------------------------- */
ft2p_t stat2p026(ftsw_t *ftsw, int ftswid, char* ss, char* state)
{
  struct timeval tv;
  ft2p_t p;

  /* retrieve time just before reading registers */
  /* read registers */
  regs2p026(ftsw, &tv, &p, &p);

  statft_headline(ftswid, p.fpgaid, p.fpgaver, &tv, ss);

  color2p026(&p, &p, ss, state);

  summary2p026(&tv, &p, &p, ss);
  statft_jitter(ftsw, FT2P_JCTL, FT2P_JREG, ss);
  return p;
}

