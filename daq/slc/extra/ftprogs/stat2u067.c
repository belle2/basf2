/*

  20150305 separated from statft.c
  20150522 X-line fix
  20150702 omask address fix
  20160106 jsta copied from ft2p026
  
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
#include "ft2u067.h"

typedef ft2u_t fast_t;
typedef ft2u_t slow_t;
typedef ft2u_t dump_t;

#include "statft.h"

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
  s->tlu        = read_ftsw(ftsw, FT2U_TLU);
  s->tluutim    = read_ftsw(ftsw, FT2U_TLUUTIM);
  s->tluctim    = read_ftsw(ftsw, FT2U_TLUCTIM);
  s->udead      = read_ftsw(ftsw, FT2U_UDEAD);
  s->cdead      = read_ftsw(ftsw, FT2U_CDEAD);
  s->pdead      = read_ftsw(ftsw, FT2U_PDEAD);
  s->edead      = read_ftsw(ftsw, FT2U_EDEAD);
  s->fdead      = read_ftsw(ftsw, FT2U_FDEAD);
  s->rdead      = read_ftsw(ftsw, FT2U_RDEAD);
  for (i=0; i<=7; i++) {
    s->odead[i]   = read_ftsw(ftsw, FT2U_ODEAD + i*4);
  };
  for (i=0; i<=3; i++) {
    s->xdead[i]   = read_ftsw(ftsw, FT2U_XDEAD + i*4);
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
  for (i=0; i<=7; i++) {
    s->dumpo[i]   = read_ftsw(ftsw, FT2U_DUMPO + i*4);
  };
  s->dumpk      = read_ftsw(ftsw, FT2U_DUMPK);
  s->dumpi      = read_ftsw(ftsw, FT2U_DUMPI);
  s->oackq      = read_ftsw(ftsw, FT2U_OACKQ);
  for (i=0; i<=9; i++) {
    s->dumpb[i]   = read_ftsw(ftsw, FT2U_DUMPB + i*4);
  };
  s->dumpc      = read_ftsw(ftsw, FT2U_DUMPC);
  s->crc        = read_ftsw(ftsw, FT2U_CRC);
  f->errsrc     = read_ftsw(ftsw, FT2U_ERRSRC);
  s->tdelay     = read_ftsw(ftsw, FT2U_TDELAY);
}
/* --------------------------------------------------------------------- *\
   dump2u067
\* --------------------------------------------------------------------- */
void
dump2u067(ftsw_t *ftsw, dump_t *d)
{
  int i;
  uint32_t dumpc = read_ftsw(ftsw, FT2U_DUMPC);
  
  if ((dumpc & 0x800) == 0) {
    write_ftsw(ftsw, FT2U_DUMPI, 1);
    usleep(100*1000); /* 100ms sleep */
  }
  
  for (i=0; i<=7; i++) {
    d->dumpo[i] = read_ftsw(ftsw, FT2U_DUMPO + i*4);
  }
  d->dumpk    = read_ftsw(ftsw, FT2U_DUMPK);  /* 88(R) dump K-symbol */
  d->dumpi    = read_ftsw(ftsw, FT2U_DUMPI);  /* 89(R) dump info */

  for (i=0; i<=7; i++) {
    d->dumpb[i] = read_ftsw(ftsw, FT2U_DUMPB + i*4); /* 90(R) dump raw input */
  }
  statft_dump(d->dumpk, d->dumpi, d->dumpo, d->dumpb);
}
/* ---------------------------------------------------------------------- * \
   summary2u067
\* ---------------------------------------------------------------------- */
double utime_old = 0;

void
summary2u067(struct timeval *tvp, fast_t *f, slow_t *s)
{
  int clkfreq;
  double tdiff, tall, trun;
  double rateall, raterun, rateout; 
  int i;
  static const char *clksrc[] = { "in", "xtal", "fmc", "pin" };
  
  /* a few things to be calculated beforehand */
  clkfreq = D(s->clkfreq,23,0) | 0x07000000;
  tdiff = tvp->tv_sec + tvp->tv_usec * 1.0e-6
    - f->utime - (double)f->ctime / clkfreq;
  tall = (double)f->utime - (double)f->rstutim;
  trun = (double)f->errutim - (double)f->rstutim;

  rateall = tall > 0 ? (f->tincnt / tall) : 0;
  //raterun = trun > 0 ? (f->atrigc / trun) : 0;
  rateout = trun > 0 ? (f->toutcnt / trun) : 0;

  char ss[256];
  /* only useful registers */
  sprintf(ss, "16  exprun=%08x exp %d run %d sub %d\n", s->exprun,
    D(s->exprun,31,22), D(s->exprun,21,8), D(s->exprun,7,0));
  strcat(g_ftstat.statft, ss);
  sprintf(ss, "17   omask=%08x s3q=%d selx/o=%d/%d x/o=%x/%02x xor=%02x\n", s->omask,
    B(s->omask,31), D(s->omask,30,28), D(s->omask,11,8),
    D(s->omask,15,12), D(s->omask,7,0), D(s->omask, 23, 16));
  strcat(g_ftstat.statft, ss);

  sprintf(ss, "1f    jsta=%08x clk=%s%s%s%s\n",
    s->jsta,
    clksrc[D(s->jsta,29,28)],
    B(s->jsta,31) ? "" : " PLL-error",
    B(s->jsta,30) ? "" : " DCM-error",
    D(s->jsta,27,24) == 0x0c ? " GOOD-CLOCK" : " no/bad-clock");
  strcat(g_ftstat.statft, ss);
    
  sprintf(ss, "20   reset=%08x%s%s", s->reset,
    B(s->reset,31) ? " no-FIFO" : "",
    B(s->reset,30) ? " auto-reset" : "");
  strcat(g_ftstat.statft, ss);
  if (B(s->reset,24)) {
    sprintf(ss, " use-TLU(delay=%d)", D(s->reset,26,25));
    strcat(g_ftstat.statft, ss);
  }
  sprintf(ss, "%s%s%s%s%s%s\n",
     B(s->reset,16) ? " regbusy" : "",
     B(s->reset,2)  ? " BOR" : "",
     B(s->reset,23) ? " EB" : "",
     B(s->reset,21) ? " PAUSED" : "",
     B(s->reset,20) ? " RUNNING" : "",
     ! B(s->reset,20) || s->tlast ? "" : "(LIMIT)");
  strcat(g_ftstat.statft, ss);

  sprintf(ss, "28  seltrg=%08x %s\n", s->seltrg, dumtrgstr(s->seltrg));
  strcat(g_ftstat.statft, ss);
  
  sprintf(ss, "2a/2b trig=%08x %08x (in:%d -> out:%d)\n",
    s->tincnt, s->toutcnt, s->tincnt, s->toutcnt);
  strcat(g_ftstat.statft, ss);

  sprintf(ss, "29/2c last=%08x %08x (limit:%d <-> last:%d)\n",
    s->tlimit, s->tlast, s->tlimit, s->tlast);
  strcat(g_ftstat.statft, ss);
  
  sprintf(ss, "2d stafifo=%08x%s%s%s%s%s%s\n", s->stafifo,
    D(s->stafifo,31,28) ? "" : " some data",
    B(s->stafifo,31)    ? " full" : "",
    B(s->stafifo,30)    ? " orun" : "",
    B(s->stafifo,29)    ? " ahi" : "",
    B(s->stafifo,28)    ? " empty" : "",
    B(s->stafifo,0)     ? " trg-enabled" : " trg-DISABLED");
  strcat(g_ftstat.statft, ss);

  sprintf(ss, "38    stat=%08x%s%s%s%s nwff(%x) X(%x/%x) O(%02x/%02x)\n", s->stat,
    B(s->stat,28) ? " BUSY" : "",
    B(s->stat,29) ? " busyin" : "",
    B(s->stat,30) ? " pipeful" : "",
    B(s->stat,31) ? " fifoerr" : "",
    D(s->stat,27,24), /* nwff */
    D(s->stat,23,20), /* xbusy */
    D(s->stat,11,8),  /* xbusysig */
    D(s->stat,19,12), /* obusy */
    D(s->stat,7,0));  /* obusysig */
  strcat(g_ftstat.statft, ss);

  sprintf(ss, "39  linkup=%08x down(%02x) alive X(%x) O(%02x) linkup X(%x) O(%02x)\n",
    s->linkup,
    D(s->linkup,31,24),
    D(s->linkup,23,20),
    D(s->linkup,19,12),
    D(s->linkup,11,8),
    D(s->linkup,7,0));
  strcat(g_ftstat.statft, ss);

  sprintf(ss, "3a   error=%08x b2ldn(%02x) plldn(%02x) X(%x) O(%02x)%s%s%s%s\n",
    s->error,
    D(s->error,31,24), D(s->error,23,16), D(s->error,11,8), D(s->error,7,0),
    B(s->error,15) ? " ERROR" : "",
    B(s->error,14) ? " errin" : "",
    B(s->error,13) ? " clkerr" : "",
    B(s->error,12) ? " trigshort" : "");
  strcat(g_ftstat.statft, ss);

  sprintf(ss, "3b linksta=%08x b2lup(%02x) pllup(%02x)",
    s->linkerr, D(s->linkerr,31,24), D(s->linkerr,23,16));
  strcat(g_ftstat.statft, ss);
  if (B(s->linkerr,15)) {
    sprintf(ss, " LINK-ERROR=%x/%02x",
      D(s->linkerr,11,8) ^ D(s->linkup,11,8),
      D(s->linkerr,7,0)  ^ D(s->linkup,7,0));
    strcat(g_ftstat.statft, ss);
  }
  sprintf(ss, "\n");
  strcat(g_ftstat.statft, ss);

  for (i=0; i<8; i++) {
    if (B(s->linkup,i)) {
      sprintf(ss, "%02x%02x%02x  %s%d%s%08x %08x %08x%s%s%s%s%s%s",
	FTSWADDR(FT2U_ODATA)+i,
        FTSWADDR(FT2U_ODATB)+i,
        FTSWADDR(FT2U_ODATC)+i,
        B(s->omask,i)?"o":"O", i, B(s->omask,i)?"!":"=",
        s->odata[i], s->odatb[i], s->odatc[i],
	D(s->odata[i],11,4) ? "" : Bs(s->odatb[i],31," busy"),
	Bs(s->odata[i],11," feeerr"),
	B(s->odata[i],1) ? Bs(s->odata[i],10," ttlost") : " ttdn",
	B(s->odata[i],3) ? Bs(s->odata[i],9," b2llost") : " b2ldn",
	Bs(s->odata[i],8," tagerr"),
	Ds(s->odata[i],7,6," fifoerr"),
	Ds(s->odata[i],5,4," seu"));
      strcat(g_ftstat.statft, ss);
      if (D(s->odata[i],3,0) == 0x0f) {
        /* dead time is not working in ft2u */
	sprintf(ss, " tag=%d(%d) cnt=%d\n",  /*  d=%4.2f%% */
	  D(s->odatb[i],27,16), /* tag */
	  D(s->odatb[i],29,28), /* diff */
	  D(s->odatb[i],15,0)); /* cntb2l */
	strcat(g_ftstat.statft, ss);
        /* f->atimec ? f->obusyc[i] / (double)f->atimec * 100 : 0); */
      } else {
	sprintf(ss, "%s%s\n",
	  B(s->odata[i],2) ? "" : " plldn",
	  B(s->odata[i],0) ? "" : " not-alive");
	strcat(g_ftstat.statft, ss);
      }
    }
  }
  
  for (i=0; i<4; i++) {
    if (B(s->linkup,i+8)) {
      sprintf(ss, "%02x/%02x   X%d=%08x %08x en=%s%s%s%s%s empty=%s%s%s%s%s",
	0x4c+i, 0x78+i, i, s->xdata[i], s->xdatb[i],
        Bs(s->xdata[i],28,"A"),
        Bs(s->xdata[i],29,"B"),
        Bs(s->xdata[i],30,"C"),
        Bs(s->xdata[i],31,"D"),
        D(s->xdata[i],31,28)?"":"none",
        Bs(s->xdata[i],16,"A"),
        Bs(s->xdata[i],17,"B"),
        Bs(s->xdata[i],18,"C"),
        Bs(s->xdata[i],19,"D"),
        D(s->xdata[i],19,16)?"":"none");
      strcat(g_ftstat.statft, ss);
      sprintf(ss, "%s%s%s%s%s%s%s%s%s%s%s",
	(D(s->xdata[i],23,20) || B(s->xdata[i],15)) ?" ful=":"",
        Bs(s->xdata[i],20,"A"),
        Bs(s->xdata[i],21,"B"),
        Bs(s->xdata[i],22,"C"),
        Bs(s->xdata[i],23,"D"),
        Bs(s->xdata[i],15,"L"),
        Ds(s->xdata[i],27,24," err="),
        Bs(s->xdata[i],24,"A"),
        Bs(s->xdata[i],25,"B"),
        Bs(s->xdata[i],26,"C"),
        Bs(s->xdata[i],27,"D"));
      strcat(g_ftstat.statft, ss);
      sprintf(ss, " d=%5.3fs\n",
	 D(s->xdead[i],31,16) + (double)D(s->xdead[i],15,0)/(clkfreq>>11));
      strcat(g_ftstat.statft, ss);
    }
  }

  sprintf(ss, "50 latency=%08x maxtrig=%d maxtime=%4.2fus\n", s->maxtrig,
    D(s->maxtrig,31,24), D(s->maxtrig,23,0) * 7.8e-3);
  strcat(g_ftstat.statft, ss);

  sprintf(ss, "52     tlu=%08x mon=%x%s%s%s%s cntrst=%d tlutag=%d\n", f->tlu,
    D(f->tlu,31,28),
    B(f->tlu,27) ? " busy" : "",
    B(f->tlu,26) ? "(non-tlu)" : "",
    B(f->tlu,25) ? " trg" : "",
    B(f->tlu,24) ? " rst" : "",
    D(f->tlu,23,16),
    D(f->tlu,15,0));
  strcat(g_ftstat.statft, ss);

  /* -- 5a..6f -- */
  sprintf(ss, "5a/5b dead=%08x %08x %1d.%1.0fs\n", f->udead, f->cdead,
    f->udead, (double)D(f->cdead,26,0) / clkfreq * 10);
  strcat(g_ftstat.statft, ss);

  sprintf(ss, "5c-5f PEFR=%08x %08x %08x %08x %3.1fs %3.1fs %3.1fs %3.1fs\n",
    s->pdead, s->edead, s->fdead, s->rdead,
    D(s->pdead,31,16) + (double)D(s->pdead,15,0)/(clkfreq>>11),
    D(s->edead,31,16) + (double)D(s->edead,15,0)/(clkfreq>>11),
    D(s->fdead,31,16) + (double)D(s->fdead,15,0)/(clkfreq>>11),
    D(s->rdead,31,16) + (double)D(s->rdead,15,0)/(clkfreq>>11));
  strcat(g_ftstat.statft, ss);

  g_ftstat.exp = D(s->exprun,31,22);
  g_ftstat.run = D(s->exprun,21,8);
  g_ftstat.sub = D(s->exprun,7,0);
  g_ftstat.tincnt = s->tincnt;
  //g_ftstat.rateall = rateall;
  //g_ftstat.atrigc = f->atrigc;
  //g_ftstat.raterun = raterun;
  g_ftstat.toutcnt = s->toutcnt;
  //g_ftstat.rateout = rateout;
  g_ftstat.tlimit = s->tlimit;
  g_ftstat.tlast = f->tlast;
  g_ftstat.tstart = f->rstutim;
  g_ftstat.trun = f->utime - f->rstutim;
  static double toutcnt_old = 0;
  static double tincnt_old = 0;
  //g_ftstat.rateout = (double)(f->toutcnt - toutcnt_old) / ((double)(f->utime - utime_old));
  //g_ftstat.ratein = (double)(f->tincnt - tincnt_old) / ((double)(f->utime - utime_old));
  double dt = f->utime;
  dt -=  utime_old;
  if (dt > 0) {
    g_ftstat.rateout = (double)(f->toutcnt - toutcnt_old) / dt;
    g_ftstat.ratein = (double)(f->tincnt - tincnt_old) / dt;
    toutcnt_old = f->toutcnt;
    tincnt_old = f->tincnt;
    utime_old = f->utime;
  } else {
    //g_ftstat.rateout = 0;
    //g_ftstat.ratein = 0;
  }
}
/* ---------------------------------------------------------------------- *\
   verbose2u067
\* ---------------------------------------------------------------------- */
void
verbose2u067(struct timeval *tvp, fast_t *f, slow_t *s)
{
  int clkfreq;
  int i;
  double tdiff;
  static const char *clksrc[] = { "in", "xtal", "fmc", "pin" };
  
  /* a few things to be calculated beforehand */
  clkfreq = D(s->clkfreq,23,0) | 0x07000000;
  tdiff = tvp->tv_sec + tvp->tv_usec * 1.0e-6
    - f->utime - (double)f->ctime / clkfreq;

  /* list of registers */
  
  P("00  ftswid=%08x (FTSW)\n", s->ftswid);
  
  P("01 cpldver=%08x version %d.%02d\n", s->cpldver,
    D(s->cpldver, 15, 8), D(s->cpldver, 7, 0));
  
  P("02    conf=%08x %s dout=%d useprgm=%d initb=%d m012=%d(%s)\n", s->conf,
    B(s->conf, 7)?"DONE":"not-programmed",
    B(s->conf, 6), /* f_dout */
    B(s->conf, 4), /* reg_useprgm */
    B(s->conf, 3), /* f_init_b */
    D(s->conf, 2, 0),
    D(s->conf, 2, 0)==7?"slave":(D(s->conf, 2, 0)==6?"master":"else"));
  
  P("04  clksel=%08x sel=%d(%s)\n", s->clksel,
    D(s->clksel,1,0),
    D(s->clksel,1,0)==1?"i":(D(s->clksel,1,0)==2?"fmc":"xtal"));

  P("10  fpgaid=%08x (%c%c%c%c)\n", s->fpgaid,
    D(s->fpgaid,31,24), D(s->fpgaid,23,16),
    D(s->fpgaid,15,8), D(s->fpgaid,7,0));

  P("11 fpgaver=%08x vme=%d id=%d ver=%d.%02d\n", s->fpgaver,
    D(s->fpgaver,31,24), D(s->fpgaver,23,16),
    D(s->fpgaver,9,0) / 100,
    D(s->fpgaver,9,0) % 100);

  P("13 clkfreq=%08x %d Hz\n", s->clkfreq, clkfreq);
  
  if (s->setutim) {
    P("12 setutim=%08x set at %s\n", s->setutim, localtimestr(s->setutim));
    P("14   utime=%08x now at %s, up for %s\n", f->utime,
      localtimestr(f->utime),
      tdiffstr(f->utime - s->setutim));
  } else {
    P("12 setutim=%08x never set\n", s->setutim);
    P("14   utime=%08x up for %s\n", f->utime, tdiffstr(f->utime));
  }
  if (abs(tvp->tv_sec - f->utime) > 1) {
    P("time difference = %d sec\n", (int)(tvp->tv_sec - f->utime));
  }

  P("15   ctime=%08x +%8.6fs (%+8.6fs)\n", f->ctime,
    (double)f->ctime / clkfreq, tdiff);

  P("16  exprun=%08x exp %d run %d\n", s->exprun,
    D(s->exprun,31,16), D(s->exprun,15,0));

  /* -- 18..1f -- */
  
  P("17   omask=%08x s3q=%d selx/o=%d/%d x/o=%x/%02x xor=%02x\n", s->omask,
    B(s->omask,31), D(s->omask,30,28), D(s->omask,11,8),
    D(s->omask,15,12), D(s->omask,7,0), D(s->omask, 23, 16));

  P("18    addr=%08x\n", s->addr);
  P("19     cmd=%08x\n", s->cmd);

  P("1a    jtag=%08x fliptdo=%02x tck/tms/tdi=%d%d%d reg=%d auto=%d en=%02x\n",
    s->jtag, D(s->jtag,23,16), B(s->jtag,14), B(s->jtag,13), B(s->jtag,12),
    B(s->jtag,9), B(s->jtag,8), D(s->jtag,7,0));

  P("1b     tdo=%08x tdo=%d\n", s->tdo, B(s->tdo,0));

  P("1e    jrst=%08x pd=%d rst=%d autophase=%d\n", s->jrst,
    B(s->jrst,31), B(s->jrst,1), B(s->jrst,0));

  P("1f    jsta=%08x clk=%s%s%s%s\n",
    s->jsta,
    clksrc[D(s->jsta,29,28)],
    B(s->jsta,31) ? "" : " PLL-error",
    B(s->jsta,30) ? "" : " DCM-error",
    D(s->jsta,27,24) == 0x0c ? " GOOD-CLOCK" : " no/bad-clock");
    
  /* -- 20..27 -- */
  
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

  P("21 rstutim=%08x reset at %s, %s ago\n", s->rstutim,
    localtimestr(s->rstutim), tdiffstr(f->utime - s->rstutim));

  {
    static const char *src[] = { "bootup", "counter", "run", "run/counter" };
    P("22 rstctim=%08x +%8.6fs by %s reset%s\n", s->rstctim,
      (double)D(s->rstctim,26,0) / clkfreq,
      src[D(s->rstctim,28,27)], B(s->rstctim,29)?" (auto)":"");
  }
  
  P("23 errutim=%08x error at %s, %s ago\n", s->errutim,
    localtimestr(s->errutim), tdiffstr(f->utime - s->errutim));

  P("24 errctim=%08x +%8.6fs\n", s->errctim,
    (double)D(s->errctim,26,0) / clkfreq);

  P("25  errsrc=%08x cntlinkup=%d errport=%x\n", s->errsrc,
    D(s->errsrc,31,16), D(s->errsrc,11,0));

  /* -- 28..2f -- */
  
  P("28  seltrg=%08x %s\n", s->seltrg, dumtrgstr(s->seltrg));
  
  P("29  tlimit=%08x (%d)\n", s->tlimit, s->tlimit);
  P("2a  tincnt=%08x (%d)\n", s->tincnt, s->tincnt);
  P("2b   tlast=%08x (%d)\n", s->tlast, s->tlast);
  P("2c toutcnt=%08x (%d)\n", s->toutcnt, s->toutcnt);
  P("2d stafifo=%08x%s%s%s%s%s%s\n", s->stafifo,
    D(s->stafifo,31,28) ? "" : " some data",
    B(s->stafifo,31)    ? " full" : "",
    B(s->stafifo,30)    ? " orun" : "",
    B(s->stafifo,29)    ? " ahi" : "",
    B(s->stafifo,28)    ? " empty" : "",
    B(s->stafifo,0)     ? " trg-enabled" : " trg-DISABLED");

  /* -- 30..37 -- */

  P("30  enstat=%08x cnt (o=%x 2=%d p=%x) %c%02x%cb2=%x\n",
    s->enstat,
    D(s->enstat,27,24),
    D(s->enstat,6,4),
    D(s->enstat,3,0),
    B(s->enstat,16) ? 'K' : 'D',
    D(s->enstat,15,8),
    B(s->enstat,17) ? ',' : '-',
    D(s->enstat,21,20));

  P("31 revopos=%08x reg=%d\n", s->revopos, D(s->revopos,10,0));
  
  P("32 revopos=%08x sta=%d cnt=%d\n", s->revopos,
    D(s->revopos,10,0), D(s->revopos,31,16));

  P("33 badrevo=%08x bad=%d norevo=%d\n", s->badrevo,
    D(s->badrevo,31,16), D(s->badrevo,15,0));


  /* -- 38..3b -- */
  
  P("38    stat=%08x%s%s%s%s nwff(%x) X(%x/%x) O(%02x/%02x)\n", s->stat,
    B(s->stat,28) ? " BUSY" : "",
    B(s->stat,29) ? " busyin" : "",
    B(s->stat,30) ? " pipeful" : "",
    B(s->stat,31) ? " fifoerr" : "",
    D(s->stat,27,24), /* nwff */
    D(s->stat,23,20), /* xbusy */
    D(s->stat,11,8),  /* xbusysig */
    D(s->stat,19,12), /* obusy */
    D(s->stat,7,0));  /* obusysig */

  P("39  linkup=%08x down(%02x) alive X(%x) O(%02x) linkup X(%x) O(%02x)\n",
    s->linkup,
    D(s->linkup,31,24),
    D(s->linkup,23,20),
    D(s->linkup,19,12),
    D(s->linkup,11,8),
    D(s->linkup,7,0));

  P("3a   error=%08x b2ldn(%02x) plldn(%02x) X(%x) O(%02x)%s%s%s%s\n",
    s->error,
    D(s->error,31,24), D(s->error,23,16), D(s->error,11,8), D(s->error,7,0),
    B(s->error,15) ? " ERROR" : "",
    B(s->error,14) ? " errin" : "",
    B(s->error,13) ? " clkerr" : "",
    B(s->error,12) ? " trigshort" : "");

  P("3b linkerr=%08x b2lup(%02x) pllup(%02x)",
    s->linkerr, D(s->linkerr,31,24), D(s->linkerr,23,16));
  if (B(s->error,15)) {
    PP(" LINK-ERROR=%x/%02x",
      D(s->linkerr,11,8) ^ D(s->linkup,11,8),
      D(s->linkerr,7,0)  ^ D(s->linkup,7,0));
  }
  PP("\n");

  /* -- 3c..4f -- */

  for (i=0; i<8; i++) {
    P("%02x%02x/%02x O%d=%08x %08x %01x-%03x-%01x-%03x",
      0x3c+i*2, 0x3d+i*2, 0x70+i, i, s->odata[i], s->odatb[i],
      D(s->odatc[i],31,28),
      D(s->odatc[i],27,16),
      D(s->odatc[i],15,12),
      D(s->odatc[i],11,0));
    if (B(s->linkup,i)) {
      PP("%s%s%s%s%s%s%s tag=%d cnt=%d\n",
	B(s->odata[i],11) ? " busy" : "",
	B(s->odata[i],10) ? " ERROR" : "",
	B(s->odata[i],9)  ? " err1" : "",
	B(s->odata[i],8)  ? " err2" : "",
	B(s->odata[i],7)  ? " tagerr" : "",
	B(s->odata[i],6)  ? " fifoerr" : "",
	B(s->odata[i],5)  ? " full" : "",
	D(s->odatb[i],31,16), /* tag */
	D(s->odatb[i],15,0)); /* cntb2l */
    } else {
      PP(" no-linkup\n");
    }
  }
  
  for (i=0; i<4; i++) {

    P("%02x/%02x   X%d=%08x %01x-%03x-%02x-%02x",
      0x4c+i, 0x78+i, i, s->xdata[i],
      D(s->xdatb[i],31,28),
      D(s->xdatb[i],27,16),
      D(s->xdatb[i],15,8),
      D(s->xdatb[i],7,0));
    if (B(s->linkup,i+8)) {
      PP(" en=%s%s%s%s%s empty=%s%s%s%s%s",
        Bs(s->xdata[i],28,"A"),
        Bs(s->xdata[i],29,"B"),
        Bs(s->xdata[i],30,"C"),
        Bs(s->xdata[i],31,"D"),
        D(s->xdata[i],31,28)?"":"none",
        Bs(s->xdata[i],16,"A"),
        Bs(s->xdata[i],17,"B"),
        Bs(s->xdata[i],18,"C"),
        Bs(s->xdata[i],19,"D"),
        D(s->xdata[i],19,16)?"":"none");
      P("%s%s%s%s%s%s%s%s%s%s%s%s",
	(D(s->xdata[i],23,20) || B(s->xdata[i],15)) ?" ful=":"",
        Bs(s->xdata[i],20,"A"),
        Bs(s->xdata[i],21,"B"),
        Bs(s->xdata[i],22,"C"),
        Bs(s->xdata[i],23,"D"),
        Bs(s->xdata[i],15,"L"),
        Ds(s->xdata[i],27,24," err="),
        Bs(s->xdata[i],24,"A"),
        Bs(s->xdata[i],25,"B"),
        Bs(s->xdata[i],26,"C"),
        Bs(s->xdata[i],27,"D"));
      PP(" d=%5.3fs\n",
	 D(s->xdead[i],31,16) + (double)D(s->xdead[i],15,0)/(clkfreq>>11));
    } else {
      PP(" no-linkup\n");
    }
  }

  /* -- 50..54 -- */
  
  P("50 latency=%08x maxtrig=%d maxtime=%4.2fus\n", s->maxtrig,
    D(s->maxtrig,31,24), D(s->maxtrig,23,0) * 7.8e-3);

  P("52     tlu=%08x mon=%x%s%s%s%s cntrst=%d tlutag=%d\n", f->tlu,
    D(f->tlu,31,28),
    B(f->tlu,27) ? " busy" : "",
    B(f->tlu,26) ? "(non-tlu)" : "",
    B(f->tlu,25) ? " trg" : "",
    B(f->tlu,24) ? " rst" : "",
    D(f->tlu,23,16),
    D(f->tlu,15,0));

  if (s->tluutim) {
    P("53 tluutim=%08x reset at %s, %s ago\n", s->tluutim,
      localtimestr(s->tluutim), tdiffstr(f->utime - s->tluutim));
    P("54 tluctim=%08x +%8.6fs\n", s->tluctim,
      (double)D(s->tluctim,26,0) / clkfreq);
  }

  /* -- 5a..6f -- */
  P("5a   udead=%08x %s\n", f->udead, tdiffstr(f->udead));
  P("5b   cdead=%08x +%8.6fs\n", f->cdead, (double)D(f->cdead,26,0) / clkfreq);

  P("5c   pdead=%08x (pipeline) %5.3fs\n", s->pdead,
    D(s->pdead,31,16) + (double)D(s->pdead,15,0)/(clkfreq>>11));
  P("5d   edead=%08x (error)    %5.3fs\n", s->edead,
    D(s->edead,31,16) + (double)D(s->edead,15,0)/(clkfreq>>11));
  P("5e   fdead=%08x (fifo)     %5.3fs\n", s->fdead,
    D(s->fdead,31,16) + (double)D(s->fdead,15,0)/(clkfreq>>11));
  P("5f   rdead=%08x (reg)      %5.3fs\n", s->rdead,
    D(s->rdead,31,16) + (double)D(s->rdead,15,0)/(clkfreq>>11));
  
  for (i=0; i<8; i++) {
    P("%02x  odead%d=%08x (O%d)       %5.3fs\n", 0x60+i, i, s->odead[i],
      i, D(s->odead[i],31,16) + (double)D(s->odead[i],15,0)/(clkfreq>>11));
  }
  for (i=0; i<4; i++) {
    P("%02x  xdead%d=%08x (O%d)       %5.3fs\n", 0x68+i, i, s->xdead[i],
      i, D(s->xdead[i],31,16) + (double)D(s->xdead[i],15,0)/(clkfreq>>11));
  }
}
/* ---------------------------------------------------------------------- *\
   color2u067
\* ---------------------------------------------------------------------- */
void
color2u067(ft2u_t *p)
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
  char ss[256];
  if (first) {
    sprintf(ss, "starting statft..."); /* white */
    sprintf(g_ftstat.state, "STARTING");
  } else if (! B(p->stafifo, 0) && B(p->reset, 21)) { /* paused */
    sprintf(ss, "PAUSED"); /* green */
    sprintf(g_ftstat.state, "PAUSED");
  } else if (B(p->reset, 20) && p->tlast == 0) { /* limit */
    sprintf(ss, "at trigger number limit"); /* green */
    sprintf(g_ftstat.state, "LIMIT");
  } else if (p->errport || D(p->errsrc,4,0)) { /* ERROR */
    if (tstart == p->rstutim) {
      sprintf(buf, "ERROR (%s -- %s for %ds)", trst, terr, (int)(t1 - t0));
    } else {
      sprintf(buf, "ERROR (at %s while not runnning)", terr);
    }
    sprintf(ss, buf); /* magenta */
    sprintf(g_ftstat.state, "ERROR");
  } else if (! B(p->stafifo, 0)) { /* trgstop */
    if (t1) {
      sprintf(buf, "READY (%s -- %s for %ds)", trst, terr, (int)(t1 - t0));
    } else if (t0) {
      sprintf(buf, "READY (reset at %s)", trst);
    } else {
      sprintf(buf, "READY");
    }
    sprintf(ss, buf); /* yellow */
    sprintf(g_ftstat.state, "READY");
  } else if ((p->seltrg & 7) == 0) { /* none trigger */
    sprintf(ss, "READY (trigger is disabled)"); /* yellow */
    sprintf(g_ftstat.state, "READY");
  } else if (p->toutcnt == toutcnt_sav && B(p->stat, 29)) { /* busy */
    sprintf(ss, "BUSY");
    sprintf(g_ftstat.state, "BUSY");
  } else if (p->toutcnt == toutcnt_sav &&
	     B(p->reset,23) && B(p->reset,16)) { /* EB */
    sprintf(ss, "BUSY from EB");
    sprintf(g_ftstat.state, "BUSY");
  } else if (p->toutcnt == toutcnt_sav &&
	     ! B(p->reset,23) && B(p->reset,16)) { /* EB */
    sprintf(ss, "EB0 is gone");
    sprintf(g_ftstat.state, "BUSY");
  } else if (p->tincnt == tincnt_sav) { /* no input */
    sprintf(ss, "no in-coming trigger"); /* green */
    sprintf(g_ftstat.state, "NOTRIGGER");
  } else if (p->toutcnt == toutcnt_sav) { /* not-updated */
    sprintf(ss, "BUSY? tout is not updated");
    sprintf(g_ftstat.state, "BUSY");
  } else {
    if (tstart != p->rstutim) tstart = p->rstutim;
    sprintf(ss, "RUNNING (about %3.1fHz since %s for %ds)",
	    (double)(p->toutcnt - toutcnt_sav) / interval, trst, dt);
    //setcolor(6, buf);
    sprintf(g_ftstat.state, "RUNNING");
  }
  strcat(g_ftstat.statft, ss);
  strcat(g_ftstat.statft, "\n");
  toutcnt_sav = p->toutcnt;
  tincnt_sav = p->tincnt;
  first = 0;
}
/* ---------------------------------------------------------------------- *\
   stat2u067
\* ---------------------------------------------------------------------- */
void
stat2u067(ftsw_t *ftsw, int ftswid)
{
  struct timeval tv;
  int fpgaver = read_ftsw(ftsw, FTSWREG_FPGAVER) & 0x3ff;
  ft2u_t u;
  
  if (fpgaver < 40) {
    P("ft2u%03d is programmed, better to upgrade to ft2u067 or above.\n",
      fpgaver);
    if (fpgaver < 40) {
      return;
    }
  }
  
  /* retrieve time just before reading registers */
  /* read registers */
  regs2u067(ftsw, &tv, &u, &u);
  if (showdump) {
    dump2u067(ftsw, &u);
    return;
  }
  
  struct tm *tp = localtime(&tv.tv_sec);
  if (showcolor) printf("\033[37;1m");
  statft_headline(ftswid, u.fpgaid, u.fpgaver, &tv);

  statft_assert(&tv, ftswid, u.conf, u.fpgaver, u.utime, u.jsta);
  
  /*
  printf("%s %d %s #%03d / ft2u%03d - %04d.%02d.%02d %02d:%02d:%02d.%03d\n",
	 "statft version",
	 VERSION,
	 "FTSW",
	 ftswid,
	 fpgaver,
	 tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday,
	 tp->tm_hour, tp->tm_min, tp->tm_sec, (int)(tv.tv_usec/1000));
  */
  
  //if (showcolor) {
  color2u067(&u);
  //}
  
  if (verbose)  {
    verbose2u067(&tv, &u, &u);
  } else {
    summary2u067(&tv, &u, &u);
  }
  if (showjitter) {
    statft_jitter(ftsw, FT2U_JCTL, FT2U_JREG);
  }
}
