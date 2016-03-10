/*

  20150305 separated from statft.c
  
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
#include "ft2u041.h"

#ifdef POCKET_TTD
#include "pocket_ttd.h"
#include "pocket_ttd_fast.h"
#include "pocket_ttd_dump.h"
typedef struct pocket_ttd slow_t;
typedef struct pocket_ttd_fast fast_t;
typedef struct pocket_ttd_dump dump_t;
#else
typedef ft2u_t fast_t;
typedef ft2u_t slow_t;
typedef ft2u_t dump_t;
#endif

#include "statft.h"

/* --------------------------------------------------------------------- *\
   regs2u041
\* --------------------------------------------------------------------- */
void
regs2u041(ftsw_t *ftsw, struct timeval *tvp, fast_t *f, slow_t *s)
{
  gettimeofday(tvp, 0);
  
  s->ftswid  = read_ftsw(ftsw, FT2U_FTSWID);  /* 00(RW) "FTSW" id */
  s->cpldver = read_ftsw(ftsw, FT2U_CPLDVER); /* 01(R)  CPLD version */
  s->conf    = read_ftsw(ftsw, FT2U_CONF);    /* 02(RW) FPGA program */
  s->cclk    = read_ftsw(ftsw, FT2U_CCLK);    /* 03(RW) FPGA CCLK */
  s->clksel  = read_ftsw(ftsw, FT2U_CLKSEL);  /* 04(RW) CLK select */
  s->fpgaid  = read_ftsw(ftsw, FT2U_FPGAID);  /* 10(RW) FPGA type */
  s->fpgaver = read_ftsw(ftsw, FT2U_FPGAVER); /* 11(R)  FPGA version */
  s->setutim = read_ftsw(ftsw, FT2U_SETUTIM); /* 12(RW) set utime */
  s->clkfreq = read_ftsw(ftsw, FT2U_CLKFREQ); /* 13(RW) # clocks/1s */
  f->utime   = read_ftsw(ftsw, FT2U_UTIME);   /* 14(R)  utime in sec */
  f->ctime   = read_ftsw(ftsw, FT2U_CTIME);   /* 15(R)  ctime in clock */
  s->exprun  = read_ftsw(ftsw, FT2U_EXPRUN);  /* 16(RW) exp/run no */
  s->omask   = read_ftsw(ftsw, FT2U_OMASK);   /* 18(RW) RJ-45 port */
  s->stamisc = read_ftsw(ftsw, FT2U_STAMISC); /* 19(R)  link status */
  s->jtag    = read_ftsw(ftsw, FT2U_JTAG);    /* 1a(RW) JTAG control */
  s->jtdo    = read_ftsw(ftsw, FT2U_JTDO);    /* 1b(R)  JTAG TDO */
  s->jctl    = read_ftsw(ftsw, FT2U_JCTL);    /* 1c(RW) jitter control */
  s->jreg    = read_ftsw(ftsw, FT2U_JREG);    /* 1d(R)  jitter register */
  s->jrst    = read_ftsw(ftsw, FT2U_JRST);    /* 1e(RW) jitter reset */
  s->jsta    = read_ftsw(ftsw, FT2U_JSTA);    /* 1f(R)  jitter status */
  s->reset   = read_ftsw(ftsw, FT2U_RESET);   /* 20(RW) reset */
  s->utimrst = read_ftsw(ftsw, FT2U_UTIMRST); /* 21(R)  reset utime */
  s->ctimrst = read_ftsw(ftsw, FT2U_CTIMRST); /* 22(R)  reset ctime */
  s->utimerr = read_ftsw(ftsw, FT2U_UTIMERR); /* 23(R)  error utime */
  s->ctimerr = read_ftsw(ftsw, FT2U_CTIMERR); /* 24(R)  error ctime */
  s->errsrc  = read_ftsw(ftsw, FT2U_ERRSRC);  /* 25(R)  error source */
  s->trgset  = read_ftsw(ftsw, FT2U_TRGSET);  /* 28(RW) gentrig set */
  s->tlimit  = read_ftsw(ftsw, FT2U_TLIMIT);  /* 29(RW) # gentrig */
  s->tincnt  = read_ftsw(ftsw, FT2U_TINCNT);  /* 2a(R)  # trigger-in */
  s->toutcnt = read_ftsw(ftsw, FT2U_TOUTCNT); /* 2b(R)  # trigger-out */
  s->tlast   = read_ftsw(ftsw, FT2U_TLAST);   /* 2c(R)  # trigger last */
  s->stafifo = read_ftsw(ftsw, FT2U_STAFIFO); /* 2d(R)  FIFO status */
  // Do not read FT2U_FIFO.
  s->enstat  = read_ftsw(ftsw, FT2U_ENSTAT);  /* 30(R)  encoder status */
  s->revopos = read_ftsw(ftsw, FT2U_REVOPOS); /* 31(RW) revosig pos */
  s->revoin  = read_ftsw(ftsw, FT2U_REVOIN);  /* 32(R)  revosig status */
  s->revocnt = read_ftsw(ftsw, FT2U_REVOCNT); /* 33(R)  bad revo */
  s->setaddr = read_ftsw(ftsw, FT2U_SETADDR); /* 34(RW) set address */
  s->setcmd  = read_ftsw(ftsw, FT2U_SETCMD);  /* 35(RW) command */
  s->lckfreq = read_ftsw(ftsw, FT2U_LCKFREQ); /* 37(R)  lck frequency */
  s->stat    = read_ftsw(ftsw, FT2U_STAT);    /* 38(R)  port status */
  s->linkup  = read_ftsw(ftsw, FT2U_LINKUP);  /* 39(R)  link status */
  s->error   = read_ftsw(ftsw, FT2U_ERROR);   /* 38(R)  port error */
  s->linkerr = read_ftsw(ftsw, FT2U_LINKERR); /* 3b(R)  link error */
  s->staa[0] = read_ftsw(ftsw, FT2U_STA0A);   /* 3c(R)  status port 1 */
  s->stab[0] = read_ftsw(ftsw, FT2U_STA0B);   /* 3d(R)  status port 1 */
  s->staa[1] = read_ftsw(ftsw, FT2U_STA1A);   /* 3e(R)  status port 2 */
  s->stab[1] = read_ftsw(ftsw, FT2U_STA1B);   /* 3f(R)  status port 2 */
  s->staa[2] = read_ftsw(ftsw, FT2U_STA2A);   /* 40(R)  status port 3 */
  s->stab[2] = read_ftsw(ftsw, FT2U_STA2B);   /* 41(R)  status port 3 */
  s->staa[3] = read_ftsw(ftsw, FT2U_STA3A);   /* 42(R)  status port 4 */
  s->stab[3] = read_ftsw(ftsw, FT2U_STA3B);   /* 43(R)  status port 4 */
  s->staa[4] = read_ftsw(ftsw, FT2U_STA4A);   /* 44(R)  status port 5 */
  s->stab[4] = read_ftsw(ftsw, FT2U_STA4B);   /* 45(R)  status port 5 */
  s->staa[5] = read_ftsw(ftsw, FT2U_STA5A);   /* 46(R)  status port 6 */
  s->stab[5] = read_ftsw(ftsw, FT2U_STA5B);   /* 47(R)  status port 6 */
  s->staa[6] = read_ftsw(ftsw, FT2U_STA6A);   /* 48(R)  status port 7 */
  s->stab[6] = read_ftsw(ftsw, FT2U_STA6B);   /* 49(R)  status port 7 */
  s->staa[7] = read_ftsw(ftsw, FT2U_STA7A);   /* 4a(R)  status port 8 */
  s->stab[7] = read_ftsw(ftsw, FT2U_STA7B);   /* 4b(R)  status port 8 */
#if 0
  s->staa[8] = read_ftsw(ftsw, FT2U_STA8A);   /* 4c(R)  status port 9 */
  s->stab[8] = read_ftsw(ftsw, FT2U_STA8B);   /* 4d(R)  status port 9 */
  s->staa[9] = read_ftsw(ftsw, FT2U_STA9A);   /* 4e(R)  status port 10 */
  s->stab[9] = read_ftsw(ftsw, FT2U_STA9B);   /* 4f(R)  status port 10 */
  s->maxt[0] = read_ftsw(ftsw, FT2U_MAXT0);   /* 50(RW) flow control 1 */
  s->maxt[1] = read_ftsw(ftsw, FT2U_MAXT1);   /* 51(RW) flow control 2 */
  s->maxt[2] = read_ftsw(ftsw, FT2U_MAXT2);   /* 52(RW) flow control 3 */
  s->maxt[3] = read_ftsw(ftsw, FT2U_MAXT3);   /* 53(RW) flow control 4 */
  s->maxt[4] = read_ftsw(ftsw, FT2U_MAXT4);   /* 54(RW) flow control 5 */
  s->maxt[5] = read_ftsw(ftsw, FT2U_MAXT5);   /* 55(RW) flow control 6 */
  s->maxt[6] = read_ftsw(ftsw, FT2U_MAXT6);   /* 56(RW) flow control 7 */
  s->maxt[7] = read_ftsw(ftsw, FT2U_MAXT7);   /* 57(RW) flow control 8 */
  s->maxt[8] = read_ftsw(ftsw, FT2U_MAXT8);   /* 58(RW) flow control 9 */
  s->maxt[9] = read_ftsw(ftsw, FT2U_MAXT9);   /* 59(RW) flow control 10 */
#endif
  s->xsta[0] = read_ftsw(ftsw, FT2U_XSTA1);   /* 4c(R)  ttrx 1 status */
  s->xsta[1] = read_ftsw(ftsw, FT2U_XSTA2);   /* 4d(R)  ttrx 2 status */
  s->xsta[2] = read_ftsw(ftsw, FT2U_XSTA3);   /* 4e(R)  ttrx 3 status */
  s->xsta[3] = read_ftsw(ftsw, FT2U_XSTA4);   /* 4f(R)  ttrx 4 status */

  s->latency = read_ftsw(ftsw, FT2U_LATENCY); /* 50(RW) flow control */
  f->tlu     = read_ftsw(ftsw, FT2U_TLU);     /* 52(RW) TLU status */
  s->utimtlu = read_ftsw(ftsw, FT2U_UTIMTLU); /* 53(RW) TLU reset utime */
  s->ctimtlu = read_ftsw(ftsw, FT2U_CTIMTLU); /* 54(RW) TLU reset ctime */

  f->udead   = read_ftsw(ftsw, FT2U_UDEAD);   /* 5a(R)  deadtime sec */
  f->cdead   = read_ftsw(ftsw, FT2U_CDEAD);   /* 5b(R)  deadtime clock */
  s->pdead   = read_ftsw(ftsw, FT2U_PDEAD);   /* 5c(R)  pipeline deadtime */
  s->edead   = read_ftsw(ftsw, FT2U_EDEAD);   /* 5d(R)  error deadtime */
  s->fdead   = read_ftsw(ftsw, FT2U_FDEAD);   /* 5e(R)  fifoful deadtime */
  s->rdead   = read_ftsw(ftsw, FT2U_RDEAD);   /* 5f(R)  software deadtime */

  s->odead[0] = read_ftsw(ftsw, FT2U_ODEAD0); /* 60(R)  port 0 deadtime */
  s->odead[1] = read_ftsw(ftsw, FT2U_ODEAD1); /* 61(R)  port 1 deadtime */
  s->odead[2] = read_ftsw(ftsw, FT2U_ODEAD2); /* 62(R)  port 2 deadtime */
  s->odead[3] = read_ftsw(ftsw, FT2U_ODEAD3); /* 63(R)  port 3 deadtime */
  s->odead[4] = read_ftsw(ftsw, FT2U_ODEAD4); /* 64(R)  port 4 deadtime */
  s->odead[5] = read_ftsw(ftsw, FT2U_ODEAD5); /* 65(R)  port 5 deadtime */
  s->odead[6] = read_ftsw(ftsw, FT2U_ODEAD6); /* 66(R)  port 6 deadtime */
  s->odead[7] = read_ftsw(ftsw, FT2U_ODEAD7); /* 67(R)  port 7 deadtime */
  s->xdead[0] = read_ftsw(ftsw, FT2U_XDEAD1); /* 68(R)  ttrx 1 deadtime */
  s->xdead[1] = read_ftsw(ftsw, FT2U_XDEAD2); /* 68(R)  ttrx 1 deadtime */
  s->xdead[2] = read_ftsw(ftsw, FT2U_XDEAD3); /* 68(R)  ttrx 1 deadtime */
  s->xdead[3] = read_ftsw(ftsw, FT2U_XDEAD4); /* 68(R)  ttrx 1 deadtime */

  s->odbg[0] = read_ftsw(ftsw, FT2U_ODBG0);   /* 70(RW) dbg port 1 */
  s->odbg[1] = read_ftsw(ftsw, FT2U_ODBG1);   /* 71(RW) dbg port 2 */
  s->odbg[2] = read_ftsw(ftsw, FT2U_ODBG2);   /* 72(RW) dbg port 3 */
  s->odbg[3] = read_ftsw(ftsw, FT2U_ODBG3);   /* 73(RW) dbg port 4 */
  s->odbg[4] = read_ftsw(ftsw, FT2U_ODBG4);   /* 74(RW) dbg port 5 */
  s->odbg[5] = read_ftsw(ftsw, FT2U_ODBG5);   /* 75(RW) dbg port 6 */
  s->odbg[6] = read_ftsw(ftsw, FT2U_ODBG6);   /* 76(RW) dbg port 7 */
  s->odbg[7] = read_ftsw(ftsw, FT2U_ODBG7);   /* 77(RW) dbg port 8 */
  s->xdbg[0] = read_ftsw(ftsw, FT2U_XDBG1);   /* 78(RW) dbg ttrx 1 */
  s->xdbg[1] = read_ftsw(ftsw, FT2U_XDBG2);   /* 79(RW) dbg ttrx 2 */
  s->xdbg[2] = read_ftsw(ftsw, FT2U_XDBG3);   /* 7a(RW) dbg ttrx 3 */
  s->xdbg[3] = read_ftsw(ftsw, FT2U_XDBG4);   /* 7b(RW) dbg ttrx 4 */

  s->tdelay  = read_ftsw(ftsw, FT2U_TDELAY);  /* 9f(RW) trigger delay */
}
/* --------------------------------------------------------------------- *\
   dump2u041
\* --------------------------------------------------------------------- */
void
dump2u041(ftsw_t *ftsw, dump_t *d)
{
  uint32_t dumpc = read_ftsw(ftsw, FT2U_DUMPC);
  
  if ((dumpc & 0x800) == 0) {
    write_ftsw(ftsw, FT2U_DUMPI, 1);
    usleep(100*1000); /* 100ms sleep */
  }

  d->dumpo[0] = read_ftsw(ftsw, FT2U_DUMPO0); /* 80(R) dump octet */
  d->dumpo[1] = read_ftsw(ftsw, FT2U_DUMPO1); /* 81(R) dump octet */
  d->dumpo[2] = read_ftsw(ftsw, FT2U_DUMPO2); /* 82(R) dump octet */
  d->dumpo[3] = read_ftsw(ftsw, FT2U_DUMPO3); /* 83(R) dump octet */
  d->dumpo[4] = read_ftsw(ftsw, FT2U_DUMPO4); /* 84(R) dump octet */
  d->dumpo[5] = read_ftsw(ftsw, FT2U_DUMPO5); /* 85(R) dump octet */
  d->dumpo[6] = read_ftsw(ftsw, FT2U_DUMPO6); /* 86(R) dump octet */
  d->dumpo[7] = read_ftsw(ftsw, FT2U_DUMPO7); /* 87(R) dump octet */
  d->dumpk    = read_ftsw(ftsw, FT2U_DUMPK);  /* 88(R) dump K-symbol */
  d->dumpi    = read_ftsw(ftsw, FT2U_DUMPI);  /* 89(R) dump info */

  d->dump[0]  = read_ftsw(ftsw, FT2U_DUMP0);  /* 90(R) dump raw input */
  d->dump[1]  = read_ftsw(ftsw, FT2U_DUMP1);  /* 91(R) dump raw input */
  d->dump[2]  = read_ftsw(ftsw, FT2U_DUMP2);  /* 92(R) dump raw input */
  d->dump[3]  = read_ftsw(ftsw, FT2U_DUMP3);  /* 93(R) dump raw input */
  d->dump[4]  = read_ftsw(ftsw, FT2U_DUMP4);  /* 94(R) dump raw input */
  d->dump[5]  = read_ftsw(ftsw, FT2U_DUMP5);  /* 95(R) dump raw input */
  d->dump[6]  = read_ftsw(ftsw, FT2U_DUMP6);  /* 96(R) dump raw input */
  d->dump[7]  = read_ftsw(ftsw, FT2U_DUMP7);  /* 97(R) dump raw input */
  d->dump[8]  = read_ftsw(ftsw, FT2U_DUMP8);  /* 98(R) dump raw input */
  d->dump[9]  = read_ftsw(ftsw, FT2U_DUMP9);  /* 99(R) dump raw input */

  statft_dump(d->dumpk, d->dumpi, d->dumpo, d->dump);
}
/* ---------------------------------------------------------------------- * \
   summary2u041
\* ---------------------------------------------------------------------- */
void
summary2u041(struct timeval *tvp, fast_t *f, slow_t *s)
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
  
  P("18   omask=%08x s3q=%d selx/o=%d/%d x/o=%x/%02x xor=%02x\n", s->omask,
    B(s->omask,31), D(s->omask,30,28), D(s->omask,11,8),
    D(s->omask,15,12), D(s->omask,7,0), D(s->omask, 23, 16));

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

  P("28  trgset=%08x %s\n", s->trgset, dumtrgstr(s->trgset));
  
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

  P("3b linksta=%08x b2lup(%02x) pllup(%02x)",
    s->linkerr, D(s->linkerr,31,24), D(s->linkerr,23,16));
  if (B(s->linkerr,15)) {
    PP(" LINK-ERROR=%x/%02x",
      D(s->linkerr,11,8) ^ D(s->linkup,11,8),
      D(s->linkerr,7,0)  ^ D(s->linkup,7,0));
  }
  PP("\n");

  for (i=0; i<8; i++) {
    if (B(s->linkup,i)) {
      P("%02x%02x/%02x O%d=%08x %08x %08x%s%s%s%s%s%s%s tag=%d(%d) cnt=%d ",
	0x3c+i*2, 0x3d+i*2, 0x70+i, i, s->staa[i], s->stab[i], s->odbg[i],
	Bs(s->stab[i],31," busy"),
	Bs(s->staa[i],11," ERROR"),
	Ds(s->staa[i],10,9," errs"),
	Bs(s->staa[i],8," tagerr"),
	Bs(s->staa[i],7," fifoerr"),
	Bs(s->staa[i],6," fifoful"),
	Ds(s->staa[i],5,4," seu"),
	D(s->stab[i],27,16), /* tag */
	D(s->stab[i],29,28), /* diff */
	D(s->stab[i],15,0)); /* cntb2l */

      PP("d=%5.3fs\n",
	 D(s->odead[i],31,16) + (double)D(s->odead[i],15,0)/(clkfreq>>11));
    }
  }
  
  for (i=0; i<4; i++) {
    if (B(s->linkup,i+8)) {
      P("%02x/%02x   X%d=%08x %08x en=%x b=%x f=%x e=%x%s ",
	0x4c+i, 0x78+i, i, s->xsta[i], s->xdbg[i],
	D(s->xsta[i],15,12),
	D(s->xsta[i],11,8),
	D(s->xsta[i],7,4),
	D(s->xsta[i],3,0),
	B(s->stat,24+i) ? " NWFF" : "");
      PP("d=%5.3fs\n",
	 D(s->xdead[i],31,16) + (double)D(s->xdead[i],15,0)/(clkfreq>>11));
    }
  }

  P("50 latency=%08x maxtrig=%d maxtime=%4.2fus\n", s->latency,
    D(s->latency,31,24), D(s->latency,23,0) * 7.8e-3);

  P("52     tlu=%08x mon=%x%s%s%s%s cntrst=%d tlutag=%d\n", f->tlu,
    D(f->tlu,31,28),
    B(f->tlu,27) ? " busy" : "",
    B(f->tlu,26) ? "(non-tlu)" : "",
    B(f->tlu,25) ? " trg" : "",
    B(f->tlu,24) ? " rst" : "",
    D(f->tlu,23,16),
    D(f->tlu,15,0));

  /* -- 5a..6f -- */
  P("5a/5b dead=%08x %08x %1d.%1.0fs\n", f->udead, f->cdead,
    f->udead, (double)D(f->cdead,26,0) / clkfreq * 10);

  P("5c-5f PEFR=%08x %08x %08x %08x %3.1fs %3.1fs %3.1fs %3.1fs\n",
    s->pdead, s->edead, s->fdead, s->rdead,
    D(s->pdead,31,16) + (double)D(s->pdead,15,0)/(clkfreq>>11),
    D(s->edead,31,16) + (double)D(s->edead,15,0)/(clkfreq>>11),
    D(s->fdead,31,16) + (double)D(s->fdead,15,0)/(clkfreq>>11),
    D(s->rdead,31,16) + (double)D(s->rdead,15,0)/(clkfreq>>11));
}
/* ---------------------------------------------------------------------- *\
   verbose2u041
\* ---------------------------------------------------------------------- */
void
verbose2u041(struct timeval *tvp, fast_t *f, slow_t *s)
{
  int clkfreq;
  int i;
  double tdiff;
  static const char *clksrc[] = { "in", "xtal", "fmc", "???" };
  
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
  
  P("18   omask=%08x s3q=%d selx/o=%d/%d x/o=%x/%02x xor=%02x\n", s->omask,
    B(s->omask,31), D(s->omask,30,28), D(s->omask,11,8),
    D(s->omask,15,12), D(s->omask,7,0), D(s->omask, 23, 16));

  P("19 stamisc=%08x dummy in FTSW2\n", s->stamisc);

  P("1a    jtag=%08x fliptdo=%02x tck/tms/tdi=%d%d%d reg=%d auto=%d en=%02x\n",
    s->jtag, D(s->jtag,23,16), B(s->jtag,14), B(s->jtag,13), B(s->jtag,12),
    B(s->jtag,9), B(s->jtag,8), D(s->jtag,7,0));

  P("1b    jtdo=%08x tdo=%d\n", s->jtdo, B(s->jtdo,0));

  P("1e    jrst=%08x pd=%d rst=%d autophase=%d\n", s->jrst,
    B(s->jrst,31), B(s->jrst,1), B(s->jrst,0));

  P("1f    jsta=%08x pll=%d dcm=%d clk=%d(%s) phase=%x retry=%d count=%d\n",
    s->jsta,
    B(s->jsta,31), B(s->jsta,30), D(s->jsta,29,28), clksrc[D(s->jsta,29,28)],
    D(s->jsta,27,24), D(s->jsta,23,16),D(s->jsta,15,0));
  WARN("phase", D(s->jsta,27,24) != 0x0c);

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

  P("21 utimrst=%08x reset at %s, %s ago\n", s->utimrst,
    localtimestr(s->utimrst), tdiffstr(f->utime - s->utimrst));

  {
    static const char *src[] = { "bootup", "counter", "run", "run/counter" };
    P("22 ctimrst=%08x +%8.6fs by %s reset%s\n", s->ctimrst,
      (double)D(s->ctimrst,26,0) / clkfreq,
      src[D(s->ctimrst,28,27)], B(s->ctimrst,29)?" (auto)":"");
  }
  
  P("23 utimerr=%08x error at %s, %s ago\n", s->utimerr,
    localtimestr(s->utimerr), tdiffstr(f->utime - s->utimerr));

  P("24 ctimerr=%08x +%8.6fs\n", s->ctimerr,
    (double)D(s->ctimerr,26,0) / clkfreq);

  P("25  errsrc=%08x cntlinkup=%d errport=%x\n", s->errsrc,
    D(s->errsrc,31,16), D(s->errsrc,11,0));

  /* -- 28..2f -- */
  
  P("28  trgset=%08x %s\n", s->trgset, dumtrgstr(s->trgset));
  
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
  
  P("32  revoin=%08x sta=%d cnt=%d\n", s->revoin,
    D(s->revopos,10,0), D(s->revopos,31,16));

  P("33 revocnt=%08x bad=%d norevo=%d\n", s->revocnt,
    D(s->revocnt,31,16), D(s->revocnt,15,0));

  P("34 setaddr=%08x\n", s->setaddr);

  P("35  setcmd=%08x\n", s->setcmd);

  P("37 lckfreq=%08x %7.5fMHz\n", s->lckfreq, s->lckfreq / 1000000.0);

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
      0x3c+i*2, 0x3d+i*2, 0x70+i, i, s->staa[i], s->stab[i],
      D(s->odbg[i],31,28),
      D(s->odbg[i],27,16),
      D(s->odbg[i],15,12),
      D(s->odbg[i],11,0));
    if (B(s->linkup,i)) {
      PP("%s%s%s%s%s%s%s tag=%d cnt=%d\n",
	B(s->staa[i],11) ? " busy" : "",
	B(s->staa[i],10) ? " ERROR" : "",
	B(s->staa[i],9)  ? " err1" : "",
	B(s->staa[i],8)  ? " err2" : "",
	B(s->staa[i],7)  ? " tagerr" : "",
	B(s->staa[i],6)  ? " fifoerr" : "",
	B(s->staa[i],5)  ? " full" : "",
	D(s->stab[i],31,16), /* tag */
	D(s->stab[i],15,0)); /* cntb2l */
    } else {
      PP(" no-linkup\n");
    }
  }
  
  for (i=0; i<4; i++) {
    P("%02x/%02x   X%d=%08x %01x-%03x-%02x-%02x",
      0x4c+i, 0x78+i, i, s->xsta[i],
      D(s->xdbg[i],31,28),
      D(s->xdbg[i],27,16),
      D(s->xdbg[i],15,8),
      D(s->xdbg[i],7,0));
    if (B(s->linkup,i+8)) {
      PP(" en=%x b=%x f=%x e=%x%s\n",
	D(s->xsta[i],15,12),
	D(s->xsta[i],11,8),
	D(s->xsta[i],7,4),
	D(s->xsta[i],3,0),
	B(s->stat,24+i) ? " NWFF" : "");
    } else {
      PP(" no-linkup\n");
    }
  }

  /* -- 50..54 -- */
  
  P("50 latency=%08x maxtrig=%d maxtime=%4.2fus\n", s->latency,
    D(s->latency,31,24), D(s->latency,23,0) * 7.8e-3);

  P("52     tlu=%08x mon=%x%s%s%s%s cntrst=%d tlutag=%d\n", f->tlu,
    D(f->tlu,31,28),
    B(f->tlu,27) ? " busy" : "",
    B(f->tlu,26) ? "(non-tlu)" : "",
    B(f->tlu,25) ? " trg" : "",
    B(f->tlu,24) ? " rst" : "",
    D(f->tlu,23,16),
    D(f->tlu,15,0));

  if (s->utimtlu) {
    P("53 utimtlu=%08x reset at %s, %s ago\n", s->utimtlu,
      localtimestr(s->utimtlu), tdiffstr(f->utime - s->utimtlu));
    P("54 ctimtlu=%08x +%8.6fs\n", s->ctimtlu,
      (double)D(s->ctimtlu,26,0) / clkfreq);
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
   color2u041
\* ---------------------------------------------------------------------- */
#ifndef POCKET_TTD
void
color2u041(ft2u_t *up)
{
  int i;
  static int toutcnt_sav = -1;
  static int tincnt_sav = -1;
  static int first = 1;
  char buf[80];
  /* desy test setup
     static char *dest[] = { "FTB0", "FTB1", "FADCC", "(FEE3)",
     "(FEE4)", "DHHC", "(FEE6)", "(FEE7)",
     "CPR010", "CPR011", "CPR012", "CPR013" };
  */
  static char *dest[] = { "FEE@O5", "FEE@O7", "FEE@O9", "FEE@O11",
                          "FEE@O13", "FEE@O15", "FEE@O17", "FEE@O19",
                          "TTRX@O1", "TTRX@O2", "TTRX@O3", "TTRX@O4" };
  if (first) {
    setcolor(7, "starting statft..."); /* white */
  } else if (! B(up->stafifo, 0) && B(up->reset, 21)) { /* paused */
    setcolor(2, "PAUSED"); /* green */
  } else if (B(up->reset, 20) && up->tlast == 0) { /* limit */
    setcolor(2, "at trigger number limit"); /* green */
  } else if (up->stat & ~0x10000000) { /* BUSY */
    strcpy(buf, "BUSY from");
    for (i=0; i<12; i++) {
      if (B(up->stat, i) || B(up->stat, i+12)) {
        sprintf(buf+strlen(buf), " %s", dest[i]);
      }
    }
    setcolor(1, buf);
  } else if (! B(up->stafifo, 0)) { /* trgstop */
    setcolor(3, "READY (i.e., probably run is stopped)"); /* yellow */
  } else if ((up->trgset & 7) == 0) { /* none trigger */
    setcolor(3, "READY (trigger is disabled)"); /* yellow */
  } else if (B(up->error, 10)) { /* ERROR */
    buf[0] = buf[1] = 0;
    if (B(up->error,14)) {
      strcpy(buf, "ERROR from");
      for (i=0; i<8; i++) {
        if (B(up->stat, i)) sprintf(buf+strlen(buf), " %s", dest[i]);
      }
      setcolor(5, buf); /* magenta */
    } else if (D(up->linkerr,11,0) ^ D(up->linkup,11,0)) {
      strcpy(buf, "B2TT-LINK-ERROR from");
      for (i=0; i<12; i++) {
        if (B(up->linkerr,i) ^ B(up->linkup,i)) {
          sprintf(buf+strlen(buf), " %s", dest[i]);
        }
      }
      setcolor(5, buf);
    } else {
      setcolor(5, "ERROR from somewhere");
    }
  } else if (up->toutcnt == toutcnt_sav &&
             B(up->reset,23) && B(up->reset,16)) { /* EB */
    setcolor(1, "BUSY from EB");
  } else if (up->toutcnt == toutcnt_sav &&
             ! B(up->reset,23) && B(up->reset,16)) { /* EB */
    setcolor(1, "EB0 is gone");
  } else if (up->tincnt == tincnt_sav) { /* no input */
    setcolor(2, "no in-coming trigger"); /* green */
  } else if (up->toutcnt == toutcnt_sav) { /* not-updated */
    setcolor(1, "BUSY? tout is not updated");
  } else {
    sprintf(buf, "RUNNING (about %3.1fHz)",
            (double)(up->toutcnt - toutcnt_sav) / interval);
    setcolor(6, buf);
  }
  toutcnt_sav = up->toutcnt;
  tincnt_sav = up->tincnt;
  first = 0;
}
#endif
/* ---------------------------------------------------------------------- *\
   stat2u041
\* ---------------------------------------------------------------------- */
#ifndef POCKET_TTD
void
stat2u041(ftsw_t *ftsw, int ftswid)
{
  struct timeval tv;
  int fpgaver = read_ftsw(ftsw, FTSWREG_FPGAVER) & 0x3ff;
  ft2u_t u;
  
  if (fpgaver < 40) {
    P("ft2u%03d is programmed, better to upgrade to ft2u041 or above.\n",
      fpgaver);
    if (fpgaver < 40) {
      return;
    }
  }
  
  /* retrieve time just before reading registers */
  /* read registers */
  regs2u041(ftsw, &tv, &u, &u);
  statft_assert(&tv, ftswid, u.conf, u.fpgaver, u.utime, u.jsta);
  
  if (showdump) {
    dump2u041(ftsw, &u);
    return;
  }
  
  struct tm *tp = localtime(&tv.tv_sec);
  if (showcolor) printf("\033[37;1m");
  statft_headline(ftswid, u.fpgaid, u.fpgaver, &tv);

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
  
  if (showcolor) {
    color2u041(&u);
  }
  
  
  if (verbose)  {
    verbose2u041(&tv, &u, &u);
  } else {
    summary2u041(&tv, &u, &u);
  }
  if (showjitter) {
    statft_jitter(ftsw, FT2U_JCTL, FT2U_JREG);
  }
}
#endif /* ! POCKET_TTD */
