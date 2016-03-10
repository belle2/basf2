/*

  code:
   w writable register, always slow update
   s readonly register, slow update
   f readonly register, fast update
   d unreadable / destructing-read register (not for struct)
   - non existing

  begin regs
  w00 ftswid
  s01 cpldver15.0 boardid31.16
  w02 conf done7 notbusy6 usrprog4 init_b3 mconf2.0
  d03 cclk
  w04 clksel1.0
  d08 sysrst
  w10 fpgaid
  s11 fpgaver15.0 boardid31.16
  w12 setutim
  w13 clkfreq23.0
  f14 utime
  f15 ctime26.0 frozen31
  w16 exprun
  -17
  w18 omask12.0 query31 clkmask20.16
  s19 reg3s25.16
  w1a jtag fliptdo20.16 seljtag9 autojtag8 enjtag4.0
  s1b tdo0 jtagerr7.3 rtdo28.16
  w1c jctl
  s1d jreg
  w1e jpd31 jreset1 jregphase0
  s1f jpll31 jdcm30 ckmux29.28 jphase27.24 jretry23.16 jcount15.0
  w20 reset nofifo31 notagerr29 selreset28 ebup23 paused21 running20
  -   busy16 genbor2
  s21 rstutim
  s22 rstctim26.0 rstsrc30.28
  s23 errutim
  s24 errctim26.0
  f25 errport12.0 errbit31.16
  -27_26
  w28 seltrg2.0 trgopt31.20 rateval17.8 rateexp7.4 notrgclr3
  w29 tlimit
  f2a tincnt
  f2b toutcnt
  f2c tlast
  f2d stafifo fifoful31 fifoorun30 fifoemp28 fifoahi25.24 trgen0
  d2e fifo
  -2f
  s30 enstat cntpayload3.0 cntbit6.4 cntoctet27.24
  w31 revopos10.0
  s32 revoin31.16 revocand10.0
  s33 badrevo31.16 norevo15.0
  w34 addr31.12 cmdhi11.0
  s35 cmd
  s36 lckfreq
  -37
  f38 busy29 fifoerr31 pipebusy30 obusy28.16 bsyin13 obsyin12.0
  f39 olinkup12.0 trigshort31 clkerr30 oalive28.16
  f3a b2lup15.8 plllk7.0 b2ldn31.24 plldn23.16
  f3b err31 errin30 b2lor29 oerr28.16 ictrl15.14 linkerr13 olinkdn12.0
  f48_3c odata addr31.12 fee11 ttdn10 b2ldn9 tage8 fifoe7 ful6 seu5.4
  -            b2lup3 plllk2 ttup1 alive0
  f56_4a odatb busy31 diff29.28 b2ltag27.16 b2lwe15.0
  -58_57
  s59 latency17.0 maxtrig31.24
  f5a udead
  f5b cdead
  f5c pdead
  f5d edead
  f5e fdead
  f5f rdead
  f6c_60 odead
  -6f_6d
  f7c_70 odatc
  -7d
  w7e omanual12.0
  w7f incdelay12.0 clrdelay28.16
  s87_80 dump8
  s88 dumpk
  s89 dumpi
  s8a oackq12.0 cntpacket31.28 bitb27.26 subb25.24 disp23.16
  -8f_8b
  s99_90 dumpb
  -9b_9a
  w9c disp31.24 autodump11 dumpwait10.4 idump3.0
  -9d
  f9e cntrst cnterrst23.20 cntrxrst19.16
  -   ttrxrst15 trgraw114 rxrst13 utset12 busysrc9.4 errsrc3.0
  w9f tdelay31.16 selila11.8 irsvsel5.4 itrgsel1.0
  end regs

*/

static const int ft2p_revision = 1;

#include <stdint.h>

/* below this line is output of ftregs.pl */

#define FT2P_FTSWID     FTSWREG(0x00)
#define FT2P_CPLDVER    FTSWREG(0x01)
#define FT2P_CONF       FTSWREG(0x02)
#define FT2P_CCLK       FTSWREG(0x03)
#define FT2P_CLKSEL     FTSWREG(0x04)
#define FT2P_SYSRST     FTSWREG(0x08)
#define FT2P_FPGAID     FTSWREG(0x10)
#define FT2P_FPGAVER    FTSWREG(0x11)
#define FT2P_SETUTIM    FTSWREG(0x12)
#define FT2P_CLKFREQ    FTSWREG(0x13)
#define FT2P_UTIME      FTSWREG(0x14)
#define FT2P_CTIME      FTSWREG(0x15)
#define FT2P_EXPRUN     FTSWREG(0x16)
#define FT2P_OMASK      FTSWREG(0x18)
#define FT2P_REG3S      FTSWREG(0x19)
#define FT2P_JTAG       FTSWREG(0x1a)
#define FT2P_TDO        FTSWREG(0x1b)
#define FT2P_JCTL       FTSWREG(0x1c)
#define FT2P_JREG       FTSWREG(0x1d)
#define FT2P_JPD        FTSWREG(0x1e)
#define FT2P_JPLL       FTSWREG(0x1f)
#define FT2P_RESET      FTSWREG(0x20)
#define FT2P_RSTUTIM    FTSWREG(0x21)
#define FT2P_RSTCTIM    FTSWREG(0x22)
#define FT2P_ERRUTIM    FTSWREG(0x23)
#define FT2P_ERRCTIM    FTSWREG(0x24)
#define FT2P_ERRPORT    FTSWREG(0x25)
#define FT2P_SELTRG     FTSWREG(0x28)
#define FT2P_TLIMIT     FTSWREG(0x29)
#define FT2P_TINCNT     FTSWREG(0x2a)
#define FT2P_TOUTCNT    FTSWREG(0x2b)
#define FT2P_TLAST      FTSWREG(0x2c)
#define FT2P_STAFIFO    FTSWREG(0x2d)
#define FT2P_FIFO       FTSWREG(0x2e)
#define FT2P_ENSTAT     FTSWREG(0x30)
#define FT2P_REVOPOS    FTSWREG(0x31)
#define FT2P_REVOIN     FTSWREG(0x32)
#define FT2P_BADREVO    FTSWREG(0x33)
#define FT2P_ADDR       FTSWREG(0x34)
#define FT2P_CMD        FTSWREG(0x35)
#define FT2P_LCKFREQ    FTSWREG(0x36)
#define FT2P_BUSY       FTSWREG(0x38)
#define FT2P_OLINKUP    FTSWREG(0x39)
#define FT2P_B2LUP      FTSWREG(0x3a)
#define FT2P_ERR        FTSWREG(0x3b)
#define FT2P_ODATA      FTSWREG(0x3c)
#define FT2P_ODATA_12   FTSWREG(0x48)
#define FT2P_ODATB      FTSWREG(0x4a)
#define FT2P_ODATB_12   FTSWREG(0x56)
#define FT2P_LATENCY    FTSWREG(0x59)
#define FT2P_UDEAD      FTSWREG(0x5a)
#define FT2P_CDEAD      FTSWREG(0x5b)
#define FT2P_PDEAD      FTSWREG(0x5c)
#define FT2P_EDEAD      FTSWREG(0x5d)
#define FT2P_FDEAD      FTSWREG(0x5e)
#define FT2P_RDEAD      FTSWREG(0x5f)
#define FT2P_ODEAD      FTSWREG(0x60)
#define FT2P_ODEAD_12   FTSWREG(0x6c)
#define FT2P_ODATC      FTSWREG(0x70)
#define FT2P_ODATC_12   FTSWREG(0x7c)
#define FT2P_OMANUAL    FTSWREG(0x7e)
#define FT2P_INCDELAY   FTSWREG(0x7f)
#define FT2P_DUMP       FTSWREG(0x80)
#define FT2P_DUMP_7     FTSWREG(0x87)
#define FT2P_DUMPK      FTSWREG(0x88)
#define FT2P_DUMPI      FTSWREG(0x89)
#define FT2P_OACKQ      FTSWREG(0x8a)
#define FT2P_DUMPB      FTSWREG(0x90)
#define FT2P_DUMPB_9    FTSWREG(0x99)
#define FT2P_DISP       FTSWREG(0x9c)
#define FT2P_CNTRST     FTSWREG(0x9e)
#define FT2P_TDELAY     FTSWREG(0x9f)


struct ft2p {
  uint32_t ftswid;     /* w00 */
  uint32_t cpldver;    /* s01 */
  uint32_t conf;       /* w02 */
  uint32_t clksel;     /* w04 */
  uint32_t fpgaid;     /* w10 */
  uint32_t fpgaver;    /* s11 */
  uint32_t setutim;    /* w12 */
  uint32_t clkfreq;    /* w13 */
  uint32_t utime;      /* f14 */
  uint32_t ctime;      /* f15 */
  uint32_t exprun;     /* w16 */
  uint32_t omask;      /* w18 */
  uint32_t reg3s;      /* s19 */
  uint32_t jtag;       /* w1a */
  uint32_t tdo;        /* s1b */
  uint32_t jctl;       /* w1c */
  uint32_t jreg;       /* s1d */
  uint32_t jpd;        /* w1e */
  uint32_t jpll;       /* s1f */
  uint32_t reset;      /* w20 */
  uint32_t rstutim;    /* s21 */
  uint32_t rstctim;    /* s22 */
  uint32_t errutim;    /* s23 */
  uint32_t errctim;    /* s24 */
  uint32_t errport;    /* f25 */
  uint32_t seltrg;     /* w28 */
  uint32_t tlimit;     /* w29 */
  uint32_t tincnt;     /* f2a */
  uint32_t toutcnt;    /* f2b */
  uint32_t tlast;      /* f2c */
  uint32_t stafifo;    /* f2d */
  uint32_t enstat;     /* s30 */
  uint32_t revopos;    /* w31 */
  uint32_t revoin;     /* s32 */
  uint32_t badrevo;    /* s33 */
  uint32_t addr;       /* w34 */
  uint32_t cmd;        /* s35 */
  uint32_t lckfreq;    /* s36 */
  uint32_t busy;       /* f38 */
  uint32_t olinkup;    /* f39 */
  uint32_t b2lup;      /* f3a */
  uint32_t err;        /* f3b */
  uint32_t odata[13];  /* f48_3c */
  uint32_t odatb[13];  /* f56_4a */
  uint32_t latency;    /* s59 */
  uint32_t udead;      /* f5a */
  uint32_t cdead;      /* f5b */
  uint32_t pdead;      /* f5c */
  uint32_t edead;      /* f5d */
  uint32_t fdead;      /* f5e */
  uint32_t rdead;      /* f5f */
  uint32_t odead[13];  /* f6c_60 */
  uint32_t odatc[13];  /* f7c_70 */
  uint32_t omanual;    /* w7e */
  uint32_t incdelay;   /* w7f */
  uint32_t dump[8];    /* s87_80 */
  uint32_t dumpk;      /* s88 */
  uint32_t dumpi;      /* s89 */
  uint32_t oackq;      /* s8a */
  uint32_t dumpb[10];  /* s99_90 */
  uint32_t disp;       /* w9c */
  uint32_t cntrst;     /* f9e */
  uint32_t tdelay;     /* w9f */
};

typedef struct ft2p ft2p_t;


#if 0
void
regs_ft2p(ftsw_t* ftsw, struct timeval* tvp, fast_t* f, slow_t* s)
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
  for (i = 0; i <= 12; i++) {
    f->odata[i]   = read_ftsw(ftsw, FT2P_ODATA + i * 4);
  };
  for (i = 0; i <= 12; i++) {
    f->odatb[i]   = read_ftsw(ftsw, FT2P_ODATB + i * 4);
  };
  s->latency    = read_ftsw(ftsw, FT2P_LATENCY);
  f->udead      = read_ftsw(ftsw, FT2P_UDEAD);
  f->cdead      = read_ftsw(ftsw, FT2P_CDEAD);
  f->pdead      = read_ftsw(ftsw, FT2P_PDEAD);
  f->edead      = read_ftsw(ftsw, FT2P_EDEAD);
  f->fdead      = read_ftsw(ftsw, FT2P_FDEAD);
  f->rdead      = read_ftsw(ftsw, FT2P_RDEAD);
  for (i = 0; i <= 12; i++) {
    f->odead[i]   = read_ftsw(ftsw, FT2P_ODEAD + i * 4);
  };
  for (i = 0; i <= 12; i++) {
    f->odatc[i]   = read_ftsw(ftsw, FT2P_ODATC + i * 4);
  };
  s->omanual    = read_ftsw(ftsw, FT2P_OMANUAL);
  s->incdelay   = read_ftsw(ftsw, FT2P_INCDELAY);
  for (i = 0; i <= 7; i++) {
    s->dump[i]    = read_ftsw(ftsw, FT2P_DUMP + i * 4);
  };
  s->dumpk      = read_ftsw(ftsw, FT2P_DUMPK);
  s->dumpi      = read_ftsw(ftsw, FT2P_DUMPI);
  s->oackq      = read_ftsw(ftsw, FT2P_OACKQ);
  for (i = 0; i <= 9; i++) {
    s->dumpb[i]   = read_ftsw(ftsw, FT2P_DUMPB + i * 4);
  };
  s->disp       = read_ftsw(ftsw, FT2P_DISP);
  f->cntrst     = read_ftsw(ftsw, FT2P_CNTRST);
  s->tdelay     = read_ftsw(ftsw, FT2P_TDELAY);
}
#endif /* 0 */
