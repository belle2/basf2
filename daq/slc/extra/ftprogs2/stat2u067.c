/*

  20150305 separated from statft.c
  20150522 X-line fix
  20150702 omask address fix
  20160106 jsta copied from ft2p026
  20160328 for pocket_ttd desy beamtest version
  20160403 errsrc added, use f->error to change color
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

#include "ft2u067.h"

typedef ft2u_t fast_t;
typedef ft2u_t slow_t;
typedef ft2u_t dump_t;
#include "ftsw.h"

#include "statft.h"

/* ---------------------------------------------------------------------- * \
   summary2u067
\* ---------------------------------------------------------------------- */
void
summary2u067(struct timeval *tvp, fast_t *f, slow_t *s, char* ss)
{
  int clkfreq;
  double tdiff;
  int i;
  static const char *clksrc[] = { "in", "xtal", "fmc", "pin" };
  char css[100];
  
  /* a few things to be calculated beforehand */
  clkfreq = D(s->clkfreq,23,0) | 0x07000000;
  tdiff = tvp->tv_sec + tvp->tv_usec * 1.0e-6
    - f->utime - (double)f->ctime / clkfreq;

  /* only useful registers */
  sprintf(css ,"16  exprun=%08x exp %d run %d sub %d\n", s->exprun,
    D(s->exprun,31,22), D(s->exprun,21,8), D(s->exprun,7,0));
  strcat(ss,css);
  
  sprintf(css ,"17   omask=%08x s3q=%d selx/o=%d/%d x/o=%x/%02x xor=%02x\n", s->omask,
    B(s->omask,31), D(s->omask,30,28), D(s->omask,11,8),
    D(s->omask,15,12), D(s->omask,7,0), D(s->omask, 23, 16));
  strcat(ss,css);

  sprintf(css ,"1f    jsta=%08x clk=%s%s%s%s\n",
    s->jsta,
    clksrc[D(s->jsta,29,28)],
    B(s->jsta,31) ? "" : " PLL-error",
    B(s->jsta,30) ? "" : " DCM-error",
    D(s->jsta,27,24) == 0x0c ? " GOOD-CLOCK" : " no/bad-clock");
  strcat(ss,css);
    
  sprintf(css ,"20   reset=%08x%s%s", s->reset,
    B(s->reset,31) ? " no-FIFO" : "",
    B(s->reset,30) ? " auto-reset" : "");
  strcat(ss,css);
  if (B(s->reset,24)) {
    sprintf(css ," use-TLU(delay=%d)", D(s->reset,26,25));
    strcat(ss,css);
  }
  sprintf(css ,"%s%s%s%s%s%s\n",
     B(s->reset,16) ? " regbusy" : "",
     B(s->reset,2)  ? " BOR" : "",
     B(s->reset,23) ? " EB" : "",
     B(s->reset,21) ? " PAUSED" : "",
     B(s->reset,20) ? " RUNNING" : "",
     ! B(s->reset,20) || f->tlast ? "" : "(LIMIT)");
  strcat(ss,css);

  /* -- 25/9e -- */
  sprintf(css ,"25/9e esrc=%08x port(%03x) bit(%04x) err(%x) busy(%02x) cnt(%d/%d)\n",
    f->errsrc,
    D(f->errport,11,0), D(f->errport,31,16),
    D(f->errsrc,3,0), D(f->errsrc,9,4),
    D(f->errsrc,23,20), D(f->errsrc,19,16));
  strcat(ss,css);

  sprintf(css ,"28  seltrg=%08x %s\n", s->seltrg, dumtrgstr(s->seltrg));
  strcat(ss,css);
  
  sprintf(css ,"2a/2b trig=%08x %08x (in:%d -> out:%d)\n",
    f->tincnt, f->toutcnt, f->tincnt, f->toutcnt);
  strcat(ss,css);
  
  sprintf(css ,"29/2c last=%08x %08x (limit:%d <-> last:%d)\n",
    s->tlimit, f->tlast, s->tlimit, f->tlast);
  strcat(ss,css);
  
  sprintf(css ,"2d stafifo=%08x%s%s%s%s%s%s\n", f->stafifo,
    D(f->stafifo,31,28) ? "" : " some data",
    B(f->stafifo,31)    ? " full" : "",
    B(f->stafifo,30)    ? " orun" : "",
    B(f->stafifo,29)    ? " ahi" : "",
    B(f->stafifo,28)    ? " empty" : "",
    B(f->stafifo,0)     ? " trg-enabled" : " trg-DISABLED");
  strcat(ss,css);

  sprintf(css ,"38    stat=%08x%s%s%s%s nwff(%x) X(%x/%x) O(%02x/%02x)\n", f->stat,
    B(f->stat,28) ? " BUSY" : "",
    B(f->stat,29) ? " busyin" : "",
    B(f->stat,30) ? " pipeful" : "",
    B(f->stat,31) ? " fifoerr" : "",
    D(f->stat,27,24), /* nwff */
    D(f->stat,23,20), /* xbusy */
    D(f->stat,11,8),  /* xbusysig */
    D(f->stat,19,12), /* obusy */
    D(f->stat,7,0));  /* obusysig */
  strcat(ss,css);

  sprintf(css ,"39  linkup=%08x down(%02x) alive X(%x) O(%02x) linkup X(%x) O(%02x)\n",
    f->linkup,
    D(f->linkup,31,24),
    D(f->linkup,23,20),
    D(f->linkup,19,12),
    D(f->linkup,11,8),
    D(f->linkup,7,0));
  strcat(ss,css);

  sprintf(css ,"3a   error=%08x b2ldn(%02x) plldn(%02x) X(%x) O(%02x)%s%s%s%s\n",
    f->error,
    D(f->error,31,24), D(f->error,23,16), D(f->error,11,8), D(f->error,7,0),
    B(f->error,15) ? " ERROR" : "",
    B(f->error,14) ? " errin" : "",
    B(f->error,13) ? " clkerr" : "",
    B(f->error,12) ? " trigshort" : "");
  strcat(ss,css);

  sprintf(css ,"3b linksta=%08x b2lup(%02x) pllup(%02x)",
    f->linkerr, D(f->linkerr,31,24), D(f->linkerr,23,16));
  strcat(ss,css);
  if (B(f->linkerr,15)) {
    sprintf(css ," LINK-ERROR=%x/%02x",
      D(f->linkerr,11,8) ^ D(f->linkup,11,8),
      D(f->linkerr,7,0)  ^ D(f->linkup,7,0));
  strcat(ss,css);
  }
  sprintf(css ,"\n");
  strcat(ss,css);

  for (i=0; i<8; i++) {
    if (B(f->linkup,i)) {
      sprintf(css ,"%02x%02x%02x  %s%d%s%08x %08x %08x%s%s%s%s%s%s%s",
	FTSWADDR(FT2U_ODATA)+i,
        FTSWADDR(FT2U_ODATB)+i,
        FTSWADDR(FT2U_ODATC)+i,
        B(s->omask,i)?"o":"O", i, B(s->omask,i)?"!":"=",
        f->odata[i], f->odatb[i], f->odatc[i],
	D(f->odata[i],11,4) ? "" : Bs(f->odatb[i],31," busy"),
	Bs(f->odata[i],11," feeerr"),
	B(f->odata[i],1) ? Bs(f->odata[i],10," ttlost") : " ttdn",
	B(f->odata[i],3) ? Bs(f->odata[i],9," b2llost") : " b2ldn",
	Bs(f->odata[i],8," tagerr"),
	Ds(f->odata[i],7,6," fifoerr"),
	Ds(f->odata[i],5,4," seu"));
      strcat(ss,css);
      if (D(f->odata[i],3,0) == 0x0f) {
        /* dead time is not working in ft2u */
	sprintf(css ," tag=%d(%d) cnt=%d\n",  /*  d=%4.2f%% */
	  D(f->odatb[i],27,16), /* tag */
	  D(f->odatb[i],29,28), /* diff */
	  D(f->odatb[i],15,0)); /* cntb2l */
	strcat(ss,css);
        /* f->atimec ? f->obusyc[i] / (double)f->atimec * 100 : 0); */
      } else {
	sprintf(css ,"%s%s\n",
	  B(f->odata[i],2) ? "" : " plldn",
	  B(f->odata[i],0) ? "" : " not-alive");
	strcat(ss,css);
      }
    }
  }
  
  for (i=0; i<4; i++) {
    if (B(f->linkup,i+8)) {
      sprintf(css ,"%02x/%02x   X%d=%08x %08x en=%s%s%s%s%s empty=%s%s%s%s%s",
	0x4c+i, 0x78+i, i, f->xdata[i], f->xdatb[i],
        Bs(f->xdata[i],28,"A"),
        Bs(f->xdata[i],29,"B"),
        Bs(f->xdata[i],30,"C"),
        Bs(f->xdata[i],31,"D"),
        D(f->xdata[i],31,28)?"":"none",
        Bs(f->xdata[i],16,"A"),
        Bs(f->xdata[i],17,"B"),
        Bs(f->xdata[i],18,"C"),
        Bs(f->xdata[i],19,"D"),
        D(f->xdata[i],19,16)?"":"none");
      strcat(ss,css);
      sprintf(css ,"%s%s%s%s%s%s%s%s%s%s%s",
	(D(f->xdata[i],23,20) || B(f->xdata[i],15)) ?" ful=":"",
        Bs(f->xdata[i],20,"A"),
        Bs(f->xdata[i],21,"B"),
        Bs(f->xdata[i],22,"C"),
        Bs(f->xdata[i],23,"D"),
        Bs(f->xdata[i],15,"L"),
        Ds(f->xdata[i],27,24," err="),
        Bs(f->xdata[i],24,"A"),
        Bs(f->xdata[i],25,"B"),
        Bs(f->xdata[i],26,"C"),
        Bs(f->xdata[i],27,"D"));
      strcat(ss,css);
      sprintf(css ," d=%5.3fs\n",
	 D(f->xdead[i],31,16) + (double)D(f->xdead[i],15,0)/(clkfreq>>11));
      strcat(ss,css);
    }
  }

  sprintf(css ,"50 latency=%08x maxtrig=%d maxtime=%4.2fus\n", s->maxtrig,
    D(s->maxtrig,31,24), D(s->maxtrig,23,0) * 7.8e-3);
  strcat(ss,css);

  sprintf(css ,"52     tlu=%08x mon=%x%s%s%s%s cntrst=%d tlutag=%d\n", f->tlu,
    D(f->tlu,31,28),
    B(f->tlu,27) ? " busy" : "",
    B(f->tlu,26) ? "(non-tlu)" : "",
    B(f->tlu,25) ? " trg" : "",
    B(f->tlu,24) ? " rst" : "",
    D(f->tlu,23,16),
    D(f->tlu,15,0));
  strcat(ss,css);

  /* -- 5a..6f -- */
  sprintf(css ,"5a/5b dead=%08x %08x %1d.%1.0fs\n", f->udead, f->cdead,
    f->udead, (double)D(f->cdead,26,0) / clkfreq * 10);
  strcat(ss,css);

  sprintf(css ,"5c-5f PEFR=%08x %08x %08x %08x %3.1fs %3.1fs %3.1fs %3.1fs\n",
    f->pdead, f->edead, f->fdead, f->rdead,
    D(f->pdead,31,16) + (double)D(f->pdead,15,0)/(clkfreq>>11),
    D(f->edead,31,16) + (double)D(f->edead,15,0)/(clkfreq>>11),
    D(f->fdead,31,16) + (double)D(f->fdead,15,0)/(clkfreq>>11),
    D(f->rdead,31,16) + (double)D(f->rdead,15,0)/(clkfreq>>11));
  strcat(ss,css);
}

/* ---------------------------------------------------------------------- *\
   color2u067
\* ---------------------------------------------------------------------- */
void color2u067(fast_t *f, slow_t *s, char* ss, char* state)
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
  strcat(ss,css);
  tp = localtime(&t1);
  sprintf(terr, "%04d.%02d.%02d %02d:%02d:%02d",
	  tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday,
	  tp->tm_hour, tp->tm_min, tp->tm_sec);
  strcat(ss,css);
  if (first) {
    sprintf(css ,"starting statft...\n"); /* white */
    strcat(ss,css);
    sprintf(state ,"STARTING"); /* white */
  } else if (! B(f->stafifo, 0) && B(s->reset, 21)) { /* paused */
    sprintf(css ,"PAUSED\n"); /* green */
    strcat(ss, css);
    sprintf(state ,"PAUSED"); /* green */
  } else if (B(s->reset, 20) && f->tlast == 0) { /* limit */
    sprintf(css ,"at trigger number limit\n"); /* green */
    strcat(ss, css);
    sprintf(state ,"PAUSED"); /* green */
  } else if (D(f->error,15,12)) { /* ERROR */
    if (tstart == s->rstutim) {
      sprintf(buf, "ERROR (%s -- %s for %ds)\n", trst, terr, (int)(t1 - t0));
    } else {
      sprintf(buf, "ERROR (at %s while not runnning)\n", terr);
    }
    sprintf(css ,buf); /* magenta */
    strcat(ss, css);
    sprintf(state ,"ERROR"); /* green */
  } else if (! B(f->stafifo, 0)) { /* trgstop */
    if (t1) {
      sprintf(buf, "READY (%s -- %s for %ds)\n", trst, terr, (int)(t1 - t0));
    } else if (t0) {
      sprintf(buf, "READY (reset at %s)\n", trst);
    } else {
      sprintf(buf, "READY");
    }
    sprintf(css ,buf); /* yellow */
    strcat(ss, css);
    sprintf(state ,"READY"); /* green */
  } else if ((s->seltrg & 7) == 0) { /* none trigger */
    sprintf(css ,"READY (trigger is disabled)\n"); /* yellow */
  } else if (f->toutcnt == toutcnt_sav && B(f->stat, 29)) { /* busy */
    sprintf(css ,"BUSY");
    strcat(ss, css);
    sprintf(state ,"BUSY"); /* green */
  } else if (f->toutcnt == toutcnt_sav &&
	     B(s->reset,23) && B(s->reset,16)) { /* EB */
    sprintf(css ,"BUSY from EB\n");
    strcat(ss, css);
    sprintf(state ,"BUSY"); /* green */
  } else if (f->toutcnt == toutcnt_sav &&
	     ! B(s->reset,23) && B(s->reset,16)) { /* EB */
    sprintf(css ,"EB0 is gone\n");
    strcat(ss, css);
    sprintf(state ,"BUSY"); /* green */
  } else if (f->tincnt == tincnt_sav) { /* no input */
    sprintf(buf, "RUNNING (no in-coming trigger, start at %s)\n", trst);
    sprintf(css ,buf); /* green */
    strcat(ss, css);
    sprintf(state ,"RUNNING"); /* green */
  } else if (f->toutcnt == toutcnt_sav) { /* not-updated */
    sprintf(css ,"BUSY? tout is not updated\n");
    strcat(ss, css);
    sprintf(state ,"BUSY"); /* green */
  } else {
    if (tstart != s->rstutim) tstart = s->rstutim;
    if (interval == 0) {
      sprintf(buf, "RUNNING (unknown freq %f since %s for %ds)\n",
              unk, trst, dt);
    } else {
      sprintf(buf, "RUNNING (about %3.1fHz since %s for %ds)\n",
              (double)(f->toutcnt - toutcnt_sav) / interval, trst, dt);
    }
    sprintf(css ,buf);
    strcat(ss, css);
    sprintf(state ,"RUNNING"); /* green */
  }
  toutcnt_sav = f->toutcnt;
  tincnt_sav = f->tincnt;
  first = 0;
}

/* --------------------------------------------------------------------- *\
   regs2u067
\* --------------------------------------------------------------------- */
void
regs2u067(ftsw_t *ftsw, struct timeval *tvp, fast_t *f, slow_t *s)
{
  int i;

  gettimeofday(tvp, 0);

  s->ftswid     = read_ftsw(ftsw, FT2U_FTSWID);
  s->cpldver    = read_ftsw(ftsw, FT2U_CPLDVER);
  s->conf       = read_ftsw(ftsw, FT2U_CONF);
  s->clksel     = read_ftsw(ftsw, FT2U_CLKSEL);
  s->fpgaid     = read_ftsw(ftsw, FT2U_FPGAID);
  s->fpgaver    = read_ftsw(ftsw, FT2U_FPGAVER);
  s->setutim    = read_ftsw(ftsw, FT2U_SETUTIM);
  s->clkfreq    = read_ftsw(ftsw, FT2U_CLKFREQ);
  f->utime      = read_ftsw(ftsw, FT2U_UTIME);
  f->ctime      = read_ftsw(ftsw, FT2U_CTIME);
  s->exprun     = read_ftsw(ftsw, FT2U_EXPRUN);
  s->omask      = read_ftsw(ftsw, FT2U_OMASK);
  s->addr       = read_ftsw(ftsw, FT2U_ADDR);
  s->cmd        = read_ftsw(ftsw, FT2U_CMD);
  s->jtag       = read_ftsw(ftsw, FT2U_JTAG);
  s->tdo        = read_ftsw(ftsw, FT2U_TDO);
  s->jctl       = read_ftsw(ftsw, FT2U_JCTL);
  s->jreg       = read_ftsw(ftsw, FT2U_JREG);
  s->jrst       = read_ftsw(ftsw, FT2U_JRST);
  s->jsta       = read_ftsw(ftsw, FT2U_JSTA);
  s->reset      = read_ftsw(ftsw, FT2U_RESET);
  s->rstutim    = read_ftsw(ftsw, FT2U_RSTUTIM);
  s->rstctim    = read_ftsw(ftsw, FT2U_RSTCTIM);
  s->errutim    = read_ftsw(ftsw, FT2U_ERRUTIM);
  s->errctim    = read_ftsw(ftsw, FT2U_ERRCTIM);
  f->errport    = read_ftsw(ftsw, FT2U_ERRPORT);
  s->seltrg     = read_ftsw(ftsw, FT2U_SELTRG);
  s->tlimit     = read_ftsw(ftsw, FT2U_TLIMIT);
  f->tincnt     = read_ftsw(ftsw, FT2U_TINCNT);
  f->toutcnt    = read_ftsw(ftsw, FT2U_TOUTCNT);
  f->tlast      = read_ftsw(ftsw, FT2U_TLAST);
  f->stafifo    = read_ftsw(ftsw, FT2U_STAFIFO);
  s->enstat     = read_ftsw(ftsw, FT2U_ENSTAT);
  s->revopos    = read_ftsw(ftsw, FT2U_REVOPOS);
  s->revocand   = read_ftsw(ftsw, FT2U_REVOCAND);
  s->badrevo    = read_ftsw(ftsw, FT2U_BADREVO);
  f->stat       = read_ftsw(ftsw, FT2U_STAT);
  f->linkup     = read_ftsw(ftsw, FT2U_LINKUP);
  f->error      = read_ftsw(ftsw, FT2U_ERROR);
  f->linkerr    = read_ftsw(ftsw, FT2U_LINKERR);
  for (i=0; i<=7; i++) {
    f->odata[i]   = read_ftsw(ftsw, FT2U_ODATA + i*4);
  };
  for (i=0; i<=7; i++) {
    f->odatb[i]   = read_ftsw(ftsw, FT2U_ODATB + i*4);
  };
  for (i=0; i<=3; i++) {
    f->xdata[i]   = read_ftsw(ftsw, FT2U_XDATA + i*4);
  };
  s->maxtrig    = read_ftsw(ftsw, FT2U_MAXTRIG);
  f->tlu        = read_ftsw(ftsw, FT2U_TLU);
  f->tluutim    = read_ftsw(ftsw, FT2U_TLUUTIM);
  f->tluctim    = read_ftsw(ftsw, FT2U_TLUCTIM);
  f->udead      = read_ftsw(ftsw, FT2U_UDEAD);
  f->cdead      = read_ftsw(ftsw, FT2U_CDEAD);
  f->pdead      = read_ftsw(ftsw, FT2U_PDEAD);
  f->edead      = read_ftsw(ftsw, FT2U_EDEAD);
  f->fdead      = read_ftsw(ftsw, FT2U_FDEAD);
  f->rdead      = read_ftsw(ftsw, FT2U_RDEAD);
  for (i=0; i<=7; i++) {
    f->odead[i]   = read_ftsw(ftsw, FT2U_ODEAD + i*4);
  };
  for (i=0; i<=3; i++) {
    f->xdead[i]   = read_ftsw(ftsw, FT2U_XDEAD + i*4);
  };
  for (i=0; i<=3; i++) {
    s->xbcnt[i]   = read_ftsw(ftsw, FT2U_XBCNT + i*4);
  };
  for (i=0; i<=7; i++) {
    f->odatc[i]   = read_ftsw(ftsw, FT2U_ODATC + i*4);
  };
  for (i=0; i<=3; i++) {
    f->xdatb[i]   = read_ftsw(ftsw, FT2U_XDATB + i*4);
  };
  s->omanual    = read_ftsw(ftsw, FT2U_OMANUAL);
  s->xmanual    = read_ftsw(ftsw, FT2U_XMANUAL);
  s->dbg        = read_ftsw(ftsw, FT2U_DBG);
  s->dbg        = read_ftsw(ftsw, FT2U_DBG);
  s->crc        = read_ftsw(ftsw, FT2U_CRC);
  f->errsrc     = read_ftsw(ftsw, FT2U_ERRSRC);
  s->tdelay     = read_ftsw(ftsw, FT2U_TDELAY);
}

/* ---------------------------------------------------------------------- *\
   stat2u067
\* ---------------------------------------------------------------------- */
ft2u_t stat2u067(ftsw_t *ftsw, int ftswid, char* ss, char* state)
{
  struct timeval tv;
  int fpgaver = read_ftsw(ftsw, FTSWREG_FPGAVER) & 0x3ff;
  ft2u_t u;
  char css[100];
  
  if (fpgaver < 40) {
      sprintf(css , "ft2u%03d is programmed, better to upgrade to ft2u067 or above.\n",
	    fpgaver);
      strcat(ss, css);
    return;
  }
  
  /* retrieve time just before reading registers */
  /* read registers */
  regs2u067(ftsw, &tv, &u, &u);
  struct tm *tp = localtime(&tv.tv_sec);
  statft_headline(ftswid, u.fpgaid, u.fpgaver, &tv, ss);
  color2u067(&u, &u, ss, state);
  summary2u067(&tv, &u, &u, ss);
  statft_jitter(ftsw, FT2U_JCTL, FT2U_JREG, ss);
  return u;
}
