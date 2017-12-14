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
  w17 omask12.0 query31 clkmask20.16
  w18 addr31.12 cmdhi11.0
  w19 cmd
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
  -26_27
  w28 seltrg2.0 trgopt31.20 rateval17.8 rateexp7.4 notrgclr3
  w29 tlimit
  f2a tincnt
  f2b toutcnt
  f2c tlast
  f2d stafifo fifoful31 fifoorun30 fifoemp28 fifoahi25.24 trgen0
  d2e fifo
  -2f

  f30 obusy
  f31 obsyin
  f32 oerrin
  f33 olinkdn
  f34 oalive
  f35 olinkup
  f36 b2ldn
  f37 plldn

  f40_4b odata addr31.12 fee11 ttdn10 b2ldn9 tage8 fifoe7 ful6 seu5.4
  -            b2lup3 plllk2 ttup1 alive0
  f54_5f odatb busy31 diff29.28 b2ltag27.16 b2lwe15.0
  f68_73 odatc
  w7c omanual12.0
  w7d clrdelay12.0
  w7e incdelay12.0
  w7f latency17.0 maxtrig31.24
  s80 lckfreq
  w81 revopos10.0
  s82 revocand10.0 revoin31.16
  s83 badrevo31.16 norevo15.0
  s84 oackq12.0 cntpacket31.28 bits27.26 subs25.24 disp23.16
  s85_87 dbg

  s88_8f dumpo
  s90_99 dumpb
  s9a dumpk
  s9b dumpi  (maybe reusable)
  w9c disp31.24 autodump11 dumpwait10.4 idump3.0
  f9e errsrc cnterrst23.20 cntrxrst19.16
  -   ttrxrst15 trgraw114 rxrst13 utset12 busysrc10.5 errsrc4.0
  w9f tdelay31.16 selila11.8 irsvsel5.4 itrgsel1.0

  fa0 atimec
  fa1 atrigc
  fa2 abusyc
  fa3 tbusyc
  fa4 cbusyc
  fa5 pbusyc
  fa6 fbusyc
  fa7 rbusyc

  sa8 reg3s25.16
  fb0_bb obusyc

  end regs

*/

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
#define FT2P_OMASK      FTSWREG(0x17)
#define FT2P_ADDR       FTSWREG(0x18)
#define FT2P_CMD        FTSWREG(0x19)
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
#define FT2P_OBUSY      FTSWREG(0x30)
#define FT2P_OBSYIN     FTSWREG(0x31)
#define FT2P_OERRIN     FTSWREG(0x32)
#define FT2P_OLINKDN    FTSWREG(0x33)
#define FT2P_OALIVE     FTSWREG(0x34)
#define FT2P_OLINKUP    FTSWREG(0x35)
#define FT2P_B2LDN      FTSWREG(0x36)
#define FT2P_PLLDN      FTSWREG(0x37)
#define FT2P_ODATA      FTSWREG(0x40)
#define FT2P_ODATA_11   FTSWREG(0x4b)
#define FT2P_ODATB      FTSWREG(0x54)
#define FT2P_ODATB_11   FTSWREG(0x5f)
#define FT2P_ODATC      FTSWREG(0x68)
#define FT2P_ODATC_11   FTSWREG(0x73)
#define FT2P_OMANUAL    FTSWREG(0x7c)
#define FT2P_CLRDELAY   FTSWREG(0x7d)
#define FT2P_INCDELAY   FTSWREG(0x7e)
#define FT2P_LATENCY    FTSWREG(0x7f)
#define FT2P_LCKFREQ    FTSWREG(0x80)
#define FT2P_REVOPOS    FTSWREG(0x81)
#define FT2P_REVOCAND   FTSWREG(0x82)
#define FT2P_BADREVO    FTSWREG(0x83)
#define FT2P_OACKQ      FTSWREG(0x84)
#define FT2P_DBG        FTSWREG(0x85)
#define FT2P_DBG_2      FTSWREG(0x87)
#define FT2P_DUMPO      FTSWREG(0x88)
#define FT2P_DUMPO_7    FTSWREG(0x8f)
#define FT2P_DUMPB      FTSWREG(0x90)
#define FT2P_DUMPB_9    FTSWREG(0x99)
#define FT2P_DUMPK      FTSWREG(0x9a)
#define FT2P_DUMPI      FTSWREG(0x9b)
#define FT2P_DISP       FTSWREG(0x9c)
#define FT2P_ERRSRC     FTSWREG(0x9e)
#define FT2P_TDELAY     FTSWREG(0x9f)
#define FT2P_ATIMEC     FTSWREG(0xa0)
#define FT2P_ATRIGC     FTSWREG(0xa1)
#define FT2P_ABUSYC     FTSWREG(0xa2)
#define FT2P_TBUSYC     FTSWREG(0xa3)
#define FT2P_CBUSYC     FTSWREG(0xa4)
#define FT2P_PBUSYC     FTSWREG(0xa5)
#define FT2P_FBUSYC     FTSWREG(0xa6)
#define FT2P_RBUSYC     FTSWREG(0xa7)
#define FT2P_REG3S      FTSWREG(0xa8)
#define FT2P_OBUSYC     FTSWREG(0xb0)
#define FT2P_OBUSYC_11  FTSWREG(0xbb)


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
  uint32_t omask;      /* w17 */
  uint32_t addr;       /* w18 */
  uint32_t cmd;        /* w19 */
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
  uint32_t obusy;      /* f30 */
  uint32_t obsyin;     /* f31 */
  uint32_t oerrin;     /* f32 */
  uint32_t olinkdn;    /* f33 */
  uint32_t oalive;     /* f34 */
  uint32_t olinkup;    /* f35 */
  uint32_t b2ldn;      /* f36 */
  uint32_t plldn;      /* f37 */
  uint32_t odata[12];  /* f40_4b */
  uint32_t odatb[12];  /* f54_5f */
  uint32_t odatc[12];  /* f68_73 */
  uint32_t omanual;    /* w7c */
  uint32_t clrdelay;   /* w7d */
  uint32_t incdelay;   /* w7e */
  uint32_t latency;    /* w7f */
  uint32_t lckfreq;    /* s80 */
  uint32_t revopos;    /* w81 */
  uint32_t revocand;   /* s82 */
  uint32_t badrevo;    /* s83 */
  uint32_t oackq;      /* s84 */
  uint32_t dbg[3];     /* s85_87 */
  uint32_t dumpo[8];   /* s88_8f */
  uint32_t dumpb[10];  /* s90_99 */
  uint32_t dumpk;      /* s9a */
  uint32_t dumpi;      /* s9b */
  uint32_t disp;       /* w9c */
  uint32_t errsrc;     /* f9e */
  uint32_t tdelay;     /* w9f */
  uint32_t atimec;     /* fa0 */
  uint32_t atrigc;     /* fa1 */
  uint32_t abusyc;     /* fa2 */
  uint32_t tbusyc;     /* fa3 */
  uint32_t cbusyc;     /* fa4 */
  uint32_t pbusyc;     /* fa5 */
  uint32_t fbusyc;     /* fa6 */
  uint32_t rbusyc;     /* fa7 */
  uint32_t reg3s;      /* sa8 */
  uint32_t obusyc[12]; /* fb0_bb */
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
  for (i = 0; i <= 11; i++) {
    f->odata[i]   = read_ftsw(ftsw, FT2P_ODATA + i * 4);
  };
  for (i = 0; i <= 11; i++) {
    f->odatb[i]   = read_ftsw(ftsw, FT2P_ODATB + i * 4);
  };
  for (i = 0; i <= 11; i++) {
    f->odatc[i]   = read_ftsw(ftsw, FT2P_ODATC + i * 4);
  };
  s->omanual    = read_ftsw(ftsw, FT2P_OMANUAL);
  s->clrdelay   = read_ftsw(ftsw, FT2P_CLRDELAY);
  s->incdelay   = read_ftsw(ftsw, FT2P_INCDELAY);
  s->latency    = read_ftsw(ftsw, FT2P_LATENCY);
  s->lckfreq    = read_ftsw(ftsw, FT2P_LCKFREQ);
  s->revopos    = read_ftsw(ftsw, FT2P_REVOPOS);
  s->revocand   = read_ftsw(ftsw, FT2P_REVOCAND);
  s->badrevo    = read_ftsw(ftsw, FT2P_BADREVO);
  s->oackq      = read_ftsw(ftsw, FT2P_OACKQ);
  for (i = 0; i <= 2; i++) {
    s->dbg[i]     = read_ftsw(ftsw, FT2P_DBG + i * 4);
  };
  for (i = 0; i <= 7; i++) {
    s->dumpo[i]   = read_ftsw(ftsw, FT2P_DUMPO + i * 4);
  };
  for (i = 0; i <= 9; i++) {
    s->dumpb[i]   = read_ftsw(ftsw, FT2P_DUMPB + i * 4);
  };
  s->dumpk      = read_ftsw(ftsw, FT2P_DUMPK);
  s->dumpi      = read_ftsw(ftsw, FT2P_DUMPI);
  s->disp       = read_ftsw(ftsw, FT2P_DISP);
  f->errsrc     = read_ftsw(ftsw, FT2P_ERRSRC);
  s->tdelay     = read_ftsw(ftsw, FT2P_TDELAY);
  f->atimec     = read_ftsw(ftsw, FT2P_ATIMEC);
  f->atrigc     = read_ftsw(ftsw, FT2P_ATRIGC);
  f->tbusyc     = read_ftsw(ftsw, FT2P_TBUSYC);
  f->cbusyc     = read_ftsw(ftsw, FT2P_CBUSYC);
  f->pbusyc     = read_ftsw(ftsw, FT2P_PBUSYC);
  f->fbusyc     = read_ftsw(ftsw, FT2P_FBUSYC);
  f->rbusyc     = read_ftsw(ftsw, FT2P_RBUSYC);
  s->reg3s      = read_ftsw(ftsw, FT2P_REG3S);
  for (i = 0; i <= 11; i++) {
    f->obusyc[i] = read_ftsw(ftsw, FT2P_OBUSYC + i * 4);
  };
}
#endif /* 0 */
