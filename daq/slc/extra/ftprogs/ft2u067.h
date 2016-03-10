/* ---------------------------------------------------------------------- *\

  ft2u067.h

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
  w17 omask7.0 xmask15.12 clkmask23.16
  w18 addr31.12 cmdhi11.0
  w19 cmd
  w1a jtag tdiblk31.24 fliptdo23.16 settck15 tck14 tms13 tdi12 tdiblk11
  -   seljtag9 autojtag8 enjtag7.0
  s1b tdo0 jtagerr7.3 statdo31.24
  w1c jctl
  s1d jreg
  w1e jrst jpd31 jreset1 jregphase0
  s1f jsta jpll31 jdcm30 ckmux29.28 jphase27.24 jretry23.16 jcount15.0
  w20 reset nofifo31 autorst30 notagerr29 selreset28 tluno1st27 tludelay26.25
  -   usetlu24 ebup23 paused21 running20 busy16 clrictl incdelay14 caldelay13
  -   setaddr12 stareset11 gtpreset10 feereset9 b2lreset8 errreset5 cntreset4
  -   trgstop3 genbor2 trgstart1 runreset0
  s21 rstutim
  s22 rstctim26.0 rstsrc30.28
  s23 errutim
  s24 errctim26.0
  f25 errport11.0 errbit31.16
  -26_27
  w28 seltrg2.0 trgopt31.20 rateval17.8 rateexp7.4 notrgclr3
  w29 tlimit
  f2a tincnt
  f2b toutcnt
  f2c tlast
  f2d stafifo fifoful31 fifoorun30 fifoemp28 fifoahi25.24 trgen0
  d2e fifo
  -2f
  s30 enstat cntpay3.0 cntbit6.4 cntoctet27.24
  w31 revopos10.0
  s32 revocand10.0 cntrevocand31.16
  s33 badrevo31.16 norevo15.0
  -34_37
  f38 stat obsyin7.0 xbsyin11.8 obusy19.12 xbusy23.20 busy28 bsyin29 pipebusy30
  -   fifoerr31
  f39 linkup olinkup7.0 xlinkup11.8 oalive19.12 xalive.23.20 olinkdn31.24
  f3a error oerr7.0 xerr11.8 trgshort12 clkerr13 errin14 staerr15
  -   plldn23.16 b2ldn31.24
  f3b linkerr15 b2lup31.24 plllk23.16 b2lor14 ictrl13.12
  f3c_43 odata addr31.12 fee11 ttdn10 b2ldn9 tage8 fifoe7 ful6 seu5.4
  -            b2lup3 plllk2 ttup1 alive0
  f44_4b odatb busy31 diff29.28 b2ltag27.16 b2lwe15.0
  f4c_4f xdata
  w50 maxtrig31.24 latency17.0
  -51
  s52 tlu
  -   tlumon31.28 tlubsy27 nontlu26 tlutrg25 tlurst24 cnttlurst23.16 tlutag15.0
  s53 tluutim
  s54 tluctim26.0
  -55_59
  s5a udead
  s5b cdead
  s5c pdead
  s5d edead
  s5e fdead
  s5f rdead
  s60_67 odead
  s68_6b xdead
  s6c_6f xbcnt
  f70_77 odatc
  f78_7b xdatb
  w7c omanual31.24 oslip23.16 oclrdelay15.8 oincdelay7.0
  w7d xmanual15.12 xclrdelay7.4 xincdelay3.0
  s7e dbg
  s7f dbgb
  s80_87 dumpo
  s88 dumpk
  s89 dumpi
  s8a oackq7.0 xackq11.8 cntpacket31.28 bits27.26 subs25.24 disp23.16
  s90_99 dumpb
  -9a_9b
  w9c dumpc disp31.24 autodump11 dumpwait10.4 idump3.0
  s9d crc31.8
  f9e errsrc3.0 cnterrst23.20 cntrxrst19.16
  -   ttrxrst15 trgraw114 rxrst13 utset12 busysrc9.4
  w9f tdelay31.16 selila11.8 irsvsel5.4 itrgsel1.0
  end regs

*/

#include <stdint.h>

#define FT2U_FTSWID     FTSWREG(0x00)
#define FT2U_CPLDVER    FTSWREG(0x01)
#define FT2U_CONF       FTSWREG(0x02)
#define FT2U_CCLK       FTSWREG(0x03)
#define FT2U_CLKSEL     FTSWREG(0x04)
#define FT2U_SYSRST     FTSWREG(0x08)
#define FT2U_FPGAID     FTSWREG(0x10)
#define FT2U_FPGAVER    FTSWREG(0x11)
#define FT2U_SETUTIM    FTSWREG(0x12)
#define FT2U_CLKFREQ    FTSWREG(0x13)
#define FT2U_UTIME      FTSWREG(0x14)
#define FT2U_CTIME      FTSWREG(0x15)
#define FT2U_EXPRUN     FTSWREG(0x16)
#define FT2U_OMASK      FTSWREG(0x17)
#define FT2U_ADDR       FTSWREG(0x18)
#define FT2U_CMD        FTSWREG(0x19)
#define FT2U_JTAG       FTSWREG(0x1a)
#define FT2U_TDO        FTSWREG(0x1b)
#define FT2U_JCTL       FTSWREG(0x1c)
#define FT2U_JREG       FTSWREG(0x1d)
#define FT2U_JRST       FTSWREG(0x1e)
#define FT2U_JSTA       FTSWREG(0x1f)
#define FT2U_RESET      FTSWREG(0x20)
#define FT2U_RSTUTIM    FTSWREG(0x21)
#define FT2U_RSTCTIM    FTSWREG(0x22)
#define FT2U_ERRUTIM    FTSWREG(0x23)
#define FT2U_ERRCTIM    FTSWREG(0x24)
#define FT2U_ERRPORT    FTSWREG(0x25)
#define FT2U_SELTRG     FTSWREG(0x28)
#define FT2U_TLIMIT     FTSWREG(0x29)
#define FT2U_TINCNT     FTSWREG(0x2a)
#define FT2U_TOUTCNT    FTSWREG(0x2b)
#define FT2U_TLAST      FTSWREG(0x2c)
#define FT2U_STAFIFO    FTSWREG(0x2d)
#define FT2U_FIFO       FTSWREG(0x2e)
#define FT2U_ENSTAT     FTSWREG(0x30)
#define FT2U_REVOPOS    FTSWREG(0x31)
#define FT2U_REVOCAND   FTSWREG(0x32)
#define FT2U_BADREVO    FTSWREG(0x33)
#define FT2U_STAT       FTSWREG(0x38)
#define FT2U_LINKUP     FTSWREG(0x39)
#define FT2U_ERROR      FTSWREG(0x3a)
#define FT2U_LINKERR    FTSWREG(0x3b)
#define FT2U_ODATA      FTSWREG(0x3c)
#define FT2U_ODATA_7    FTSWREG(0x43)
#define FT2U_ODATB      FTSWREG(0x44)
#define FT2U_ODATB_7    FTSWREG(0x4b)
#define FT2U_XDATA      FTSWREG(0x4c)
#define FT2U_XDATA_3    FTSWREG(0x4f)
#define FT2U_MAXTRIG    FTSWREG(0x50)
#define FT2U_TLU        FTSWREG(0x52)
#define FT2U_TLUUTIM    FTSWREG(0x53)
#define FT2U_TLUCTIM    FTSWREG(0x54)
#define FT2U_UDEAD      FTSWREG(0x5a)
#define FT2U_CDEAD      FTSWREG(0x5b)
#define FT2U_PDEAD      FTSWREG(0x5c)
#define FT2U_EDEAD      FTSWREG(0x5d)
#define FT2U_FDEAD      FTSWREG(0x5e)
#define FT2U_RDEAD      FTSWREG(0x5f)
#define FT2U_ODEAD      FTSWREG(0x60)
#define FT2U_ODEAD_7    FTSWREG(0x67)
#define FT2U_XDEAD      FTSWREG(0x68)
#define FT2U_XDEAD_3    FTSWREG(0x6b)
#define FT2U_XBCNT      FTSWREG(0x6c)
#define FT2U_XBCNT_3    FTSWREG(0x6f)
#define FT2U_ODATC      FTSWREG(0x70)
#define FT2U_ODATC_7    FTSWREG(0x77)
#define FT2U_XDATB      FTSWREG(0x78)
#define FT2U_XDATB_3    FTSWREG(0x7b)
#define FT2U_OMANUAL    FTSWREG(0x7c)
#define FT2U_XMANUAL    FTSWREG(0x7d)
#define FT2U_DBG        FTSWREG(0x7e)
#define FT2U_DBGB       FTSWREG(0x7f)
#define FT2U_DUMPO      FTSWREG(0x80)
#define FT2U_DUMPO_7    FTSWREG(0x87)
#define FT2U_DUMPK      FTSWREG(0x88)
#define FT2U_DUMPI      FTSWREG(0x89)
#define FT2U_OACKQ      FTSWREG(0x8a)
#define FT2U_DUMPB      FTSWREG(0x90)
#define FT2U_DUMPB_9    FTSWREG(0x99)
#define FT2U_DUMPC      FTSWREG(0x9c)
#define FT2U_CRC        FTSWREG(0x9d)
#define FT2U_ERRSRC     FTSWREG(0x9e)
#define FT2U_TDELAY     FTSWREG(0x9f)


struct ft2u {
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
  uint32_t jrst;       /* w1e */
  uint32_t jsta;       /* s1f */
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
  uint32_t revocand;   /* s32 */
  uint32_t badrevo;    /* s33 */
  uint32_t stat;       /* f38 */
  uint32_t linkup;     /* f39 */
  uint32_t error;      /* f3a */
  uint32_t linkerr;    /* f3b */
  uint32_t odata[8];   /* f3c_43 */
  uint32_t odatb[8];   /* f44_4b */
  uint32_t xdata[4];   /* f4c_4f */
  uint32_t maxtrig;    /* w50 */
  uint32_t tlu;        /* s52 */
  uint32_t tluutim;    /* s53 */
  uint32_t tluctim;    /* s54 */
  uint32_t udead;      /* s5a */
  uint32_t cdead;      /* s5b */
  uint32_t pdead;      /* s5c */
  uint32_t edead;      /* s5d */
  uint32_t fdead;      /* s5e */
  uint32_t rdead;      /* s5f */
  uint32_t odead[8];   /* s60_67 */
  uint32_t xdead[4];   /* s68_6b */
  uint32_t xbcnt[4];   /* s6c_6f */
  uint32_t odatc[8];   /* f70_77 */
  uint32_t xdatb[4];   /* f78_7b */
  uint32_t omanual;    /* w7c */
  uint32_t xmanual;    /* w7d */
  uint32_t dbg;        /* s7e */
  uint32_t dbgb;       /* s7f */
  uint32_t dumpo[8];   /* s80_87 */
  uint32_t dumpk;      /* s88 */
  uint32_t dumpi;      /* s89 */
  uint32_t oackq;      /* s8a */
  uint32_t dumpb[10];  /* s90_99 */
  uint32_t dumpc;      /* w9c */
  uint32_t crc;        /* s9d */
  uint32_t errsrc;     /* f9e */
  uint32_t tdelay;     /* w9f */
};

typedef struct ft2u ft2u_t;


#if 0
void
regs_ft2u(ftsw_t* ftsw, struct timeval* tvp, fast_t* f, slow_t* s)
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
  for (i = 0; i <= 7; i++) {
    f->odata[i]   = read_ftsw(ftsw, FT2U_ODATA + i * 4);
  };
  for (i = 0; i <= 7; i++) {
    f->odatb[i]   = read_ftsw(ftsw, FT2U_ODATB + i * 4);
  };
  for (i = 0; i <= 3; i++) {
    f->xdata[i]   = read_ftsw(ftsw, FT2U_XDATA + i * 4);
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
  for (i = 0; i <= 7; i++) {
    s->odead[i]   = read_ftsw(ftsw, FT2U_ODEAD + i * 4);
  };
  for (i = 0; i <= 3; i++) {
    s->xdead[i]   = read_ftsw(ftsw, FT2U_XDEAD + i * 4);
  };
  for (i = 0; i <= 3; i++) {
    s->xbcnt[i]   = read_ftsw(ftsw, FT2U_XBCNT + i * 4);
  };
  for (i = 0; i <= 7; i++) {
    f->odatc[i]   = read_ftsw(ftsw, FT2U_ODATC + i * 4);
  };
  for (i = 0; i <= 3; i++) {
    f->xdatb[i]   = read_ftsw(ftsw, FT2U_XDATB + i * 4);
  };
  s->omanual    = read_ftsw(ftsw, FT2U_OMANUAL);
  s->xmanual    = read_ftsw(ftsw, FT2U_XMANUAL);
  s->dbg        = read_ftsw(ftsw, FT2U_DBG);
  s->dbgb       = read_ftsw(ftsw, FT2U_DBGB);
  for (i = 0; i <= 7; i++) {
    s->dumpo[i]   = read_ftsw(ftsw, FT2U_DUMPO + i * 4);
  };
  s->dumpk      = read_ftsw(ftsw, FT2U_DUMPK);
  s->dumpi      = read_ftsw(ftsw, FT2U_DUMPI);
  s->oackq      = read_ftsw(ftsw, FT2U_OACKQ);
  for (i = 0; i <= 9; i++) {
    s->dumpb[i]   = read_ftsw(ftsw, FT2U_DUMPB + i * 4);
  };
  s->dumpc      = read_ftsw(ftsw, FT2U_DUMPC);
  s->crc        = read_ftsw(ftsw, FT2U_CRC);
  f->errsrc     = read_ftsw(ftsw, FT2U_ERRSRC);
  s->tdelay     = read_ftsw(ftsw, FT2U_TDELAY);
}
#endif /* 0 */
