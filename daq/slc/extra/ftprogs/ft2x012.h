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
  -12
  s13 clkfreq23.0
  f14 utime
  f15 ctime26.0 stapd31 sigpd30
  -17_16
  w18 xmask19.0
  -1b_19
  w1c jctl
  s1d jreg
  w1e jpd31 jreset1 jregphase0
  s1f jpll31 jdcm30 ckmux29.28 jphase27.24 jretry23.16 jcount15.0
  w20 reset notagerr29 selreset28 paused21 running20 busy16
  s21 rstutim
  s22 rstctim26.0 rstsrc30.28
  s23 errutim
  s24 errctim26.0
  f25 errport19.0
  -27_26
  f28 ttup31 clkup30 bitddr15 cntbitb14.12 bitb11.10 comma9 isk8 octet7.0
  -2c_29
  w2d handbusy16 handerr18.17 regdbg31.24
  -2f_2e
  f30 xalive19.0
  f31 xlinkup19.0
  f32 xlinkdn19.0
  f33 xbusy19.0
  f34 xbsyin19.0
  f35 xerr19.0
  -39_36
  f3a busy29 bsyin28 err15 errin14 clkerr13
  f4f_3c xdath en31.28 err27.24 ful23.20 emp19.16 nwff15 xlink14
  f63_50 xdatl bitb23.22 cntb21.20 errb19.18 linkdn11 width17.12 delay10.4
  -      iddr3.2 linkup1 alive0
  f77_64 xbcnt slowbsy31.16 fastbsy15.0
  w78 xmanual19.0 selila28.24
  w79 clrdelay19.0
  w7a incdelay19.0
  w7d xackq
  end regs

*/

static const int ft2x_revision = 1;

#include <stdint.h>

/* below this line is output of ftregs.pl */

#define FT2X_FTSWID     FTSWREG(0x00)
#define FT2X_CPLDVER    FTSWREG(0x01)
#define FT2X_CONF       FTSWREG(0x02)
#define FT2X_CCLK       FTSWREG(0x03)
#define FT2X_CLKSEL     FTSWREG(0x04)
#define FT2X_SYSRST     FTSWREG(0x08)
#define FT2X_FPGAID     FTSWREG(0x10)
#define FT2X_FPGAVER    FTSWREG(0x11)
#define FT2X_CLKFREQ    FTSWREG(0x13)
#define FT2X_UTIME      FTSWREG(0x14)
#define FT2X_CTIME      FTSWREG(0x15)
#define FT2X_XMASK      FTSWREG(0x18)
#define FT2X_JCTL       FTSWREG(0x1c)
#define FT2X_JREG       FTSWREG(0x1d)
#define FT2X_JPD        FTSWREG(0x1e)
#define FT2X_JPLL       FTSWREG(0x1f)
#define FT2X_RESET      FTSWREG(0x20)
#define FT2X_RSTUTIM    FTSWREG(0x21)
#define FT2X_RSTCTIM    FTSWREG(0x22)
#define FT2X_ERRUTIM    FTSWREG(0x23)
#define FT2X_ERRCTIM    FTSWREG(0x24)
#define FT2X_ERRPORT    FTSWREG(0x25)
#define FT2X_TTUP       FTSWREG(0x28)
#define FT2X_HANDBUSY   FTSWREG(0x2d)
#define FT2X_XALIVE     FTSWREG(0x30)
#define FT2X_XLINKUP    FTSWREG(0x31)
#define FT2X_XLINKDN    FTSWREG(0x32)
#define FT2X_XBUSY      FTSWREG(0x33)
#define FT2X_XBSYIN     FTSWREG(0x34)
#define FT2X_XERR       FTSWREG(0x35)
#define FT2X_BUSY       FTSWREG(0x3a)
#define FT2X_XDATH      FTSWREG(0x3c)
#define FT2X_XDATH_19   FTSWREG(0x4f)
#define FT2X_XDATL      FTSWREG(0x50)
#define FT2X_XDATL_19   FTSWREG(0x63)
#define FT2X_XBCNT      FTSWREG(0x64)
#define FT2X_XBCNT_19   FTSWREG(0x77)
#define FT2X_XMANUAL    FTSWREG(0x78)
#define FT2X_CLRDELAY   FTSWREG(0x79)
#define FT2X_INCDELAY   FTSWREG(0x7a)
#define FT2X_XACKQ      FTSWREG(0x7d)


struct ft2x {
  uint32_t ftswid;     /* w00 */
  uint32_t cpldver;    /* s01 */
  uint32_t conf;       /* w02 */
  uint32_t clksel;     /* w04 */
  uint32_t fpgaid;     /* w10 */
  uint32_t fpgaver;    /* s11 */
  uint32_t clkfreq;    /* s13 */
  uint32_t utime;      /* f14 */
  uint32_t ctime;      /* f15 */
  uint32_t xmask;      /* w18 */
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
  uint32_t ttup;       /* f28 */
  uint32_t handbusy;   /* w2d */
  uint32_t xalive;     /* f30 */
  uint32_t xlinkup;    /* f31 */
  uint32_t xlinkdn;    /* f32 */
  uint32_t xbusy;      /* f33 */
  uint32_t xbsyin;     /* f34 */
  uint32_t xerr;       /* f35 */
  uint32_t busy;       /* f3a */
  uint32_t xdath[20];  /* f4f_3c */
  uint32_t xdatl[20];  /* f63_50 */
  uint32_t xbcnt[20];  /* f77_64 */
  uint32_t xmanual;    /* w78 */
  uint32_t clrdelay;   /* w79 */
  uint32_t incdelay;   /* w7a */
  uint32_t xackq;      /* w7d */
};

typedef struct ft2x ft2x_t;


#if 0
void
regs_ft2x(ftsw_t* ftsw, struct timeval* tvp, fast_t* f, slow_t* s)
{
  int i;

  gettimeofday(tvp, 0);

  s->ftswid     = read_ftsw(ftsw, FT2X_FTSWID);
  s->cpldver    = read_ftsw(ftsw, FT2X_CPLDVER);
  s->conf       = read_ftsw(ftsw, FT2X_CONF);
  s->clksel     = read_ftsw(ftsw, FT2X_CLKSEL);
  s->fpgaid     = read_ftsw(ftsw, FT2X_FPGAID);
  s->fpgaver    = read_ftsw(ftsw, FT2X_FPGAVER);
  s->clkfreq    = read_ftsw(ftsw, FT2X_CLKFREQ);
  f->utime      = read_ftsw(ftsw, FT2X_UTIME);
  f->ctime      = read_ftsw(ftsw, FT2X_CTIME);
  s->xmask      = read_ftsw(ftsw, FT2X_XMASK);
  s->jctl       = read_ftsw(ftsw, FT2X_JCTL);
  s->jreg       = read_ftsw(ftsw, FT2X_JREG);
  s->jpd        = read_ftsw(ftsw, FT2X_JPD);
  s->jpll       = read_ftsw(ftsw, FT2X_JPLL);
  s->reset      = read_ftsw(ftsw, FT2X_RESET);
  s->rstutim    = read_ftsw(ftsw, FT2X_RSTUTIM);
  s->rstctim    = read_ftsw(ftsw, FT2X_RSTCTIM);
  s->errutim    = read_ftsw(ftsw, FT2X_ERRUTIM);
  s->errctim    = read_ftsw(ftsw, FT2X_ERRCTIM);
  f->errport    = read_ftsw(ftsw, FT2X_ERRPORT);
  f->ttup       = read_ftsw(ftsw, FT2X_TTUP);
  s->handbusy   = read_ftsw(ftsw, FT2X_HANDBUSY);
  f->xalive     = read_ftsw(ftsw, FT2X_XALIVE);
  f->xlinkup    = read_ftsw(ftsw, FT2X_XLINKUP);
  f->xlinkdn    = read_ftsw(ftsw, FT2X_XLINKDN);
  f->xbusy      = read_ftsw(ftsw, FT2X_XBUSY);
  f->xbsyin     = read_ftsw(ftsw, FT2X_XBSYIN);
  f->xerr       = read_ftsw(ftsw, FT2X_XERR);
  f->busy       = read_ftsw(ftsw, FT2X_BUSY);
  for (i = 0; i <= 19; i++) {
    f->xdath[i]   = read_ftsw(ftsw, FT2X_XDATH + i * 4);
  };
  for (i = 0; i <= 19; i++) {
    f->xdatl[i]   = read_ftsw(ftsw, FT2X_XDATL + i * 4);
  };
  for (i = 0; i <= 19; i++) {
    f->xbcnt[i]   = read_ftsw(ftsw, FT2X_XBCNT + i * 4);
  };
  s->xmanual    = read_ftsw(ftsw, FT2X_XMANUAL);
  s->clrdelay   = read_ftsw(ftsw, FT2X_CLRDELAY);
  s->incdelay   = read_ftsw(ftsw, FT2X_INCDELAY);
  s->xackq      = read_ftsw(ftsw, FT2X_XACKQ);
}
#endif /* 0 */
