
const int pocket_ttd_revision = 5;

struct pocket_ttd {
  uchar isrunning;
  uchar sigpipe;
  uchar verbose;
  uchar ebisbusy;
  /* tmp */
  int16 b2ebport;
  int16 b2ebsock;
  int32 b2ebip;
  int32 myip;
  int32 exp_number;
  int32 run_number;
  int32 run_count;
  /* int32 evt_number; */
  /* int32 evt_total; */
  int32 evt_number;
  int32 evt_total;
  /* FTSW registers */
  int32 ftswid;  /* FTSWREG_FTSWID  0x00(RW) "FTSW" id */
  int32 cpldver; /* FTSWREG_CPLDVER 0x01(R)  CPLD version */
  int32 conf;    /* FTSWREG_CONF    0x02(RW) FPGA program */
  int32 cclk;    /* FTSWREG_CCLK    0x03(RW) FPGA CCLK */
  int32 fpgaid;  /* FTSWREG_FPGAID  0x10(RW) FPGA type */
  int32 fpgaver; /* FTSWREG_FPGAVER 0x11(R)  FPGA version */
  int32 setutim; /* FTSWREG_SETUTIM 0x12(RW) set utime */
  int32 clkfreq; /* FTSWREG_CLKFREQ 0x13(RW) # clocks/1s */
  /* int32 utime;   /* FTSWREG_UTIME   0x14(R)  utime in sec */
  /* int32 ctime;   /* FTSWREG_CTIME   0x15(R)  ctime in clock */
  /* int32 udead;   /* FTSWREG_UDEAD   0x14(R)  deadtime sec */
  /* int32 cdead;   /* FTSWREG_CDEAD   0x15(R)  deadtime clock */
  int32 utime;   /* FTSWREG_UTIME   0x14(R)  utime in sec */
  int32 ctime;   /* FTSWREG_CTIME   0x15(R)  ctime in clock */
  int32 exprun;  /* FTSWREG_EXPRUN  0x16(RW) 10-bit exp & 22-bit run */
  int32 omask;   /* FTSWREG_OMASK   0x18(RW) RJ-45 port */
  int32 stamisc; /* FTSWREG_STAMISC 0x19(R)  link status */
  int32 jtag;    /* FTSWREG_JTAG    0x1a(RW) JTAG control */
  int32 jtdo;    /* FTSWREG_JTDO    0x1b(R)  JTAG TDO */
  int32 jctl;    /* FTSWREG_JCTL    0x1c(RW) jitter control */
  int32 jreg;    /* FTSWREG_JREG    0x1d(R)  jitter register */
  int32 jrst;    /* FTSWREG_JRST    0x1e(RW) jitter reset */
  int32 jsta;    /* FTSWREG_JSTA    0x1f(R)  jitter status */
  int32 reset;   /* FTSWREG_RESET   0x20(RW) reset */
  int32 utimrst; /* FTSWREG_UTIMRST 0x21(R)  reset utime */
  int32 ctmirst; /* FTSWREG_CTMIRST 0x22(R)  reset ctime */
  int32 utimerr; /* FTSWREG_UTIMERR 0x23(R)  error utime */
  int32 ctmierr; /* FTSWREG_CTMIERR 0x24(R)  error ctime */
  int32 errsrc;  /* FTSWREG_ERRSRC  0x25(R)  error source */
  int32 tluctrl; /* FTSWREG_TLUCTRL 0x26(RW) TLU control */
  int32 tlustat; /* FTSWREG_TLUSTAT 0x27(R)  TLU status */
  int32 trgset;  /* FTSWREG_TRGSET  0x28(RW) gentrig set */
  int32 tlimit;  /* FTSWREG_TLIMIT  0x29(RW) # gentrig */
  int32 tincnt;  /* FTSWREG_TINCNT  0x2a(R)  # trigger-in */
  int32 toutcnt; /* FTSWREG_TOUTCNT 0x2b(R)  # trigger-out */
  int32 tlast;   /* FTSWREG_TLAST   0x2c(R)  # trigger last */
  int32 stafifo; /* FTSWREG_STAFIFO 0x2d(R)  FIFO status */
  int32 enstat;  /* FTSWREG_ENSTAT  0x30(R)  encoder status */
  int32 revopos; /* FTSWREG_REVOPOS 0x31(RW) revosig pos */
  int32 revoin;  /* FTSWREG_REVOIN  0x32(R)  revosig status */
  int32 revocnt; /* FTSWREG_REVOCNT 0x33(R)  bad revo */
  int32 setaddr; /* FTSWREG_SETADDR 0x34(RW) set address */
  int32 setcmd;  /* FTSWREG_SETCMD  0x35(RW) command */
  int32 lckfreq; /* FTSWREG_LCKFREQ 0x37(R)  lck frequency */
  int32 stat;    /* FTSWREG_STAT    0x38(R)  port status */
  int32 linkup;  /* FTSWREG_LINKUP  0x39(R)  link status */
  int32 error;   /* FTSWREG_ERROR   0x38(R)  port error */
  int32 acksig;  /* FTSWREG_ACKSIG  0x3b(R)  raw ACK signal */
  int32 staa[10]; /* FTSWREG_STA[0..9]A  0x3c..0x4e(R)  status port 1-10 */
  int32 stab[10]; /* FTSWREG_STA[0..9]B  0x3d..0x4f(R)  status port 1-10 */
  int32 maxt[10]; /* FTSWREG_MAXT[0..9]  0x50..0x59(RW) flow control 1-10 */
  int32 udead;   /* FTSWREG_UDEAD   0x5a(R)  deadtime sec */
  int32 cdead;   /* FTSWREG_CDEAD   0x5b(R)  deadtime clock */
  /* following dead time has (utime[15:0] << 16 | ctime[26:11]) */
  int32 pdead;   /* FTSWREG_PDEAD   0x5c(R)  pipeline deadtime */
  int32 edead;   /* FTSWREG_EDEAD   0x5d(R)  error deadtime */
  int32 fdead;   /* FTSWREG_FDEAD   0x5e(R)  FTSW fifoful deadtime */
  int32 rdead;   /* FTSWREG_RDEAD   0x5f(R)  software (sendtoeb) deadtime */
  int32 odead[8]; /* FTSWREG_ODEAD   0x60-67(R)  port 1-8 deadtime */
  int32 xdead[4]; /* FTSWREG_XDEAD   0x68-67(R)  port 1-8 deadtime */
  int32 delay;    /* FTSWREG_DELAY   0x9f(RW)  delay */
};
