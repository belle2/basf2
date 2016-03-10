/*

  ft3d006.h for ft3[dr]006 thru ft3[dr]011
  (register map is changed from ft3[dr]012)

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
  -13_12
  f14 utime
  f15 ctime26.0 sigpd30 stapd31
  -17_16
  w18 omask9.0
  s19 suser23 sver21.16
  -1d_1a
  w1e jpd31 jreset1 jregphase0
  s1f jpll31 jdcm30 ckmux29.28 jphase27.24 jretry23.16 jcount15.0
  f20 ttup31 clkup30 bitddr15 cntbitb14.12 bitb11.10 comma9 isk8 octet7.0
  f21 trgtag
  f22 trgtyp31.28
  s23 errutim
  s24 errctim26.0
  f25 errport9.0 errbit31.16
  -2c_26
  w2d handbusy16 handerr18.17 regdbg31.24
  -37_2e
  f38 busy bsyin26 errin25 clkerr24 linkerr22 b2lor21 obusy19.10 obsyin9.0
  f39 olinkup9.0 oalive19.10 olinkdn29.20
  f3a b2ldn29.20 plldn19.10 oerrin9.0
  f3b b2lup19.10 plllk9.0
  f45_3c odata addr31.12 fee11 ttdn10 b2ldn9 tage8 fifoe7 ful6 seu5.4
  -            b2lup3 plllk2 ttup1 alive0
  f4f_46 odatb busy31 diff29.28 b2ltag27.16 b2lwe15.0
  -6f_50
  f79_70 odatc
  -7b_7a
  w7c omanual29.20 clrdelay19.10 incdelay9.0
  -7f_7d
  s87_80 dump8
  s88 dumpk
  s89 dumpi
  s8a oackq12.0 cntpacket31.28 bitb27.26 subb25.24 disp23.16
  -8f_8b
  s99_90 dumpb
  -9e_9a
  w9f selila11.8 oaux1.0
  end regs

*/

static const int ft3d_revision = 1;

#include <stdint.h>

/* below this line is output of ftregs.pl */

#define FT3D_NUMO 10

#define FT3D_FTSWID     FTSWREG(0x00)
#define FT3D_CPLDVER    FTSWREG(0x01)
#define FT3D_CONF       FTSWREG(0x02)
#define FT3D_CCLK       FTSWREG(0x03)
#define FT3D_CLKSEL     FTSWREG(0x04)
#define FT3D_SYSRST     FTSWREG(0x08)
#define FT3D_FPGAID     FTSWREG(0x10)
#define FT3D_FPGAVER    FTSWREG(0x11)
#define FT3D_UTIME      FTSWREG(0x14)
#define FT3D_CTIME      FTSWREG(0x15)
#define FT3D_OMASK      FTSWREG(0x18)
#define FT3D_REG3S      FTSWREG(0x19)
#define FT3D_JPD        FTSWREG(0x1e)
#define FT3D_JPLL       FTSWREG(0x1f)
#define FT3D_TTUP       FTSWREG(0x20)
#define FT3D_TRGTAG     FTSWREG(0x21)
#define FT3D_TRGTYP     FTSWREG(0x22)
#define FT3D_ERRUTIM    FTSWREG(0x23)
#define FT3D_ERRCTIM    FTSWREG(0x24)
#define FT3D_ERRPORT    FTSWREG(0x25)
#define FT3D_HANDBUSY   FTSWREG(0x2d)
#define FT3D_BUSY       FTSWREG(0x38)
#define FT3D_OLINKUP    FTSWREG(0x39)
#define FT3D_B2LDN      FTSWREG(0x3a)
#define FT3D_B2LUP      FTSWREG(0x3b)
#define FT3D_ODATA      FTSWREG(0x3c)
#define FT3D_ODATA_MAX  FTSWREG(0x3c + FT3D_NUMO - 1)
#define FT3D_ODATB      FTSWREG(0x46)
#define FT3D_ODATB_MAX  FTSWREG(0x46 + FT3D_NUMO - 1)
#define FT3D_ODATC      FTSWREG(0x70)
#define FT3D_ODATC_MAX  FTSWREG(0x70 + FT3D_NUMO - 1)
#define FT3D_OMANUAL    FTSWREG(0x7c)
#define FT3D_DUMP       FTSWREG(0x80)
#define FT3D_DUMP_7     FTSWREG(0x87)
#define FT3D_DUMPK      FTSWREG(0x88)
#define FT3D_DUMPI      FTSWREG(0x89)
#define FT3D_OACKQ      FTSWREG(0x8a)
#define FT3D_DUMPB      FTSWREG(0x90)
#define FT3D_DUMPB_9    FTSWREG(0x99)
#define FT3D_SELILA     FTSWREG(0x9f)


struct ft3d {
  uint32_t ftswid;     /* w00 */
  uint32_t cpldver;    /* s01 */
  uint32_t conf;       /* w02 */
  uint32_t clksel;     /* w04 */
  uint32_t fpgaid;     /* w10 */
  uint32_t fpgaver;    /* s11 */
  uint32_t setutim;    /* w12 */
  uint32_t utime;      /* f14 */
  uint32_t ctime;      /* f15 */
  uint32_t omask;      /* w18 */
  uint32_t reg3s;      /* s19 */
  uint32_t jtag;       /* w1a */
  uint32_t tdo;        /* s1b */
  uint32_t jctl;       /* w1c */
  uint32_t jreg;       /* s1d */
  uint32_t jpd;        /* w1e */
  uint32_t jpll;       /* s1f */
  uint32_t ttup;       /* f20 */
  uint32_t trgtag;     /* f21 */
  uint32_t trgtyp;     /* f22 */
  uint32_t errutim;    /* s23 */
  uint32_t errctim;    /* s24 */
  uint32_t errport;    /* f25 */
  uint32_t handbusy;   /* w2d */
  uint32_t busy;       /* f38 */
  uint32_t olinkup;    /* f39 */
  uint32_t b2ldn;      /* f3a */
  uint32_t b2lup;      /* f3b */
  uint32_t odata[FT3D_NUMO];  /* f45_3c */
  uint32_t odatb[FT3D_NUMO];  /* f4f_46 */
  uint32_t odatc[FT3D_NUMO];  /* f79_70 */
  uint32_t omanual;    /* w7c */
  uint32_t dump[8];    /* s87_80 */
  uint32_t dumpk;      /* s88 */
  uint32_t dumpi;      /* s89 */
  uint32_t oackq;      /* s8a */
  uint32_t dumpb[10];  /* s99_90 */
  uint32_t selila;     /* w9f */
};

typedef struct ft3d ft3d_t;


#if 0
void
regs_ft3d(ftsw_t* ftsw, struct timeval* tvp, fast_t* f, slow_t* s)
{
  int i;

  gettimeofday(tvp, 0);

  s->ftswid     = read_ftsw(ftsw, FT3D_FTSWID);
  s->cpldver    = read_ftsw(ftsw, FT3D_CPLDVER);
  s->conf       = read_ftsw(ftsw, FT3D_CONF);
  s->clksel     = read_ftsw(ftsw, FT3D_CLKSEL);
  s->fpgaid     = read_ftsw(ftsw, FT3D_FPGAID);
  s->fpgaver    = read_ftsw(ftsw, FT3D_FPGAVER);
  s->setutim    = read_ftsw(ftsw, FT3D_SETUTIM);
  f->utime      = read_ftsw(ftsw, FT3D_UTIME);
  f->ctime      = read_ftsw(ftsw, FT3D_CTIME);
  s->omask      = read_ftsw(ftsw, FT3D_OMASK);
  s->reg3s      = read_ftsw(ftsw, FT3D_REG3S);
  s->jtag       = read_ftsw(ftsw, FT3D_JTAG);
  s->tdo        = read_ftsw(ftsw, FT3D_TDO);
  s->jctl       = read_ftsw(ftsw, FT3D_JCTL);
  s->jreg       = read_ftsw(ftsw, FT3D_JREG);
  s->jpd        = read_ftsw(ftsw, FT3D_JPD);
  s->jpll       = read_ftsw(ftsw, FT3D_JPLL);
  f->ttup       = read_ftsw(ftsw, FT3D_TTUP);
  f->trgtag     = read_ftsw(ftsw, FT3D_TRGTAG);
  f->trgtyp     = read_ftsw(ftsw, FT3D_TRGTYP);
  s->errutim    = read_ftsw(ftsw, FT3D_ERRUTIM);
  s->errctim    = read_ftsw(ftsw, FT3D_ERRCTIM);
  f->errport    = read_ftsw(ftsw, FT3D_ERRPORT);
  s->handbusy   = read_ftsw(ftsw, FT3D_HANDBUSY);
  s->dbga       = read_ftsw(ftsw, FT3D_DBGA);
  s->dbgb       = read_ftsw(ftsw, FT3D_DBGB);
  f->busy       = read_ftsw(ftsw, FT3D_BUSY);
  f->olinkup    = read_ftsw(ftsw, FT3D_OLINKUP);
  f->b2ldn      = read_ftsw(ftsw, FT3D_B2LDN);
  f->b2lup      = read_ftsw(ftsw, FT3D_B2LUP);
  for (i = 0; i <= 9; i++) {
    f->odata[i]   = read_ftsw(ftsw, FT3D_ODATA + i * 4);
  };
  for (i = 0; i <= 9; i++) {
    f->odatb[i]   = read_ftsw(ftsw, FT3D_ODATB + i * 4);
  };
  for (i = 0; i <= 9; i++) {
    f->odatc[i]   = read_ftsw(ftsw, FT3D_ODATC + i * 4);
  };
  s->omanual    = read_ftsw(ftsw, FT3D_OMANUAL);
  for (i = 0; i <= 7; i++) {
    s->dump[i]    = read_ftsw(ftsw, FT3D_DUMP + i * 4);
  };
  s->dumpk      = read_ftsw(ftsw, FT3D_DUMPK);
  s->dumpi      = read_ftsw(ftsw, FT3D_DUMPI);
  s->oackq      = read_ftsw(ftsw, FT3D_OACKQ);
  for (i = 0; i <= 9; i++) {
    s->dumpb[i]   = read_ftsw(ftsw, FT3D_DUMPB + i * 4);
  };
  s->selila     = read_ftsw(ftsw, FT3D_SELILA);
}
#endif /* 0 */
