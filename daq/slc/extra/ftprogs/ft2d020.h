/*

  ft2d020.h for ft[23][dr]020 and on
  (register map is changed from ft[23][dr]020)

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
  -12_13
  f14 utime
  f15 ctime26.0 sigpd30 stapd31
  s16 exprun
  s17 dmask19.0 seldout31.27
  s18 addr
  s19 suser23 sver21.16
  -1a_1d
  w1e jpd31 jreset1 jregphase0
  s1f jpll31 jdcm30 ckmux29.28 jphase27.24 jretry23.16 jcount15.0
  f20 ttup31 clkup30 trig29 runreset28 cntbit14.12
  f21 trgtag
  f22 trgtyp31.28
  s23 errutim
  s24 errctim26.0
  f25 errport9.0 errbit31.16
  f26 errbit31.16 derrbit15.0
  -26_2c
  w2d regdbg31.24 handbusy16 handerr18.17
  -2e_2f
  f30 busy26 errin25 clkerr24 obsyin9.0
  f31 obsyin9.0
  f32 oerrin9.0
  f33 olinkdn9.0
  f34 oalive9.0
  f35 olinkup9.0
  f36 b2ldn9.0
  f37 plldn9.0
  -38_3f
  f40_4b odata addr31.12 fee11 ttdn10 b2ldn9 tage8 fifoe7 ful6 seu5.4
  -            b2lup3 plllk2 ttup1 alive0
  -4c_53
  f54_5f odatb busy31 diff29.28 b2ltag27.16 b2lwe15.0
  -60_67
  f68_73 odatc
  -74_7b
  w7c omanual9.0
  w7d clrdelay9.0
  w7e incdelay9.0
  -7f_83
  s84 bits27.0
  -85_87
  s88_8f dumpo
  s99_90 dumpb
  s9a dumpk
  s9b dumpi
  w9c idump3.0 dumpwait10.4 autodump11 disp31.24
  -94
  -8f_8b
  s9e vio13.8
  w9f selila11.8 oaux1.0
  end regs

*/

static const int ft2d_revision = 4;

#include <stdint.h>

/* below this line is output of ftregs.pl */

#ifndef FT2D_NUMO
#define FT2D_NUMO 12
#endif

#ifndef FT3D_NUMO
#define FT3D_NUMO 10
#endif

#define FT2D_FTSWID     FTSWREG(0x00)
#define FT2D_CPLDVER    FTSWREG(0x01)
#define FT2D_CONF       FTSWREG(0x02)
#define FT2D_CCLK       FTSWREG(0x03)
#define FT2D_CLKSEL     FTSWREG(0x04)
#define FT2D_SYSRST     FTSWREG(0x08)
#define FT2D_FPGAID     FTSWREG(0x10)
#define FT2D_FPGAVER    FTSWREG(0x11)
#define FT2D_UTIME      FTSWREG(0x14)
#define FT2D_CTIME      FTSWREG(0x15)
#define FT2D_EXPRUN     FTSWREG(0x16)
#define FT2D_DMASK      FTSWREG(0x17)
#define FT2D_ADDR       FTSWREG(0x18)
#define FT2D_SUSER      FTSWREG(0x19)
#define FT2D_JPD        FTSWREG(0x1e)
#define FT2D_JPLL       FTSWREG(0x1f)
#define FT2D_TTUP       FTSWREG(0x20)
#define FT2D_TRGTAG     FTSWREG(0x21)
#define FT2D_TRGTYP     FTSWREG(0x22)
#define FT2D_ERRUTIM    FTSWREG(0x23)
#define FT2D_ERRCTIM    FTSWREG(0x24)
#define FT2D_ERRPORT    FTSWREG(0x25)
#define FT2D_ERRBIT     FTSWREG(0x26)
#define FT2D_REGDBG     FTSWREG(0x2d)
#define FT2D_BUSY       FTSWREG(0x30)
#define FT2D_OBSYIN     FTSWREG(0x31)
#define FT2D_OERRIN     FTSWREG(0x32)
#define FT2D_OLINKDN    FTSWREG(0x33)
#define FT2D_OALIVE     FTSWREG(0x34)
#define FT2D_OLINKUP    FTSWREG(0x35)
#define FT2D_B2LDN      FTSWREG(0x36)
#define FT2D_PLLDN      FTSWREG(0x37)
#define FT2D_ODATA      FTSWREG(0x40)
#define FT2D_ODATA_9    FTSWREG(0x49)
#define FT2D_ODATB      FTSWREG(0x54)
#define FT2D_ODATB_9    FTSWREG(0x5d)
#define FT2D_ODATC      FTSWREG(0x68)
#define FT2D_ODATC_9    FTSWREG(0x71)
#define FT2D_OMANUAL    FTSWREG(0x7c)
#define FT2D_CLRDELAY   FTSWREG(0x7d)
#define FT2D_INCDELAY   FTSWREG(0x7e)
#define FT2D_BITS       FTSWREG(0x84)
#define FT2D_DUMPO      FTSWREG(0x88)
#define FT2D_DUMPO_7    FTSWREG(0x8f)
#define FT2D_DUMPB      FTSWREG(0x90)
#define FT2D_DUMPB_9    FTSWREG(0x99)
#define FT2D_DUMPK      FTSWREG(0x9a)
#define FT2D_DUMPI      FTSWREG(0x9b)
#define FT2D_IDUMP      FTSWREG(0x9c)
#define FT2D_VIO        FTSWREG(0x9e)
#define FT2D_SELILA     FTSWREG(0x9f)


struct ft2d {
  uint32_t ftswid;     /* w00 */
  uint32_t cpldver;    /* s01 */
  uint32_t conf;       /* w02 */
  uint32_t clksel;     /* w04 */
  uint32_t fpgaid;     /* w10 */
  uint32_t fpgaver;    /* s11 */
  uint32_t utime;      /* f14 */
  uint32_t ctime;      /* f15 */
  uint32_t exprun;     /* s16 */
  uint32_t dmask;      /* s17 */
  uint32_t addr;       /* s18 */
  uint32_t suser;      /* s19 */
  uint32_t jpd;        /* w1e */
  uint32_t jpll;       /* s1f */
  uint32_t ttup;       /* f20 */
  uint32_t trgtag;     /* f21 */
  uint32_t trgtyp;     /* f22 */
  uint32_t errutim;    /* s23 */
  uint32_t errctim;    /* s24 */
  uint32_t errport;    /* f25 */
  uint32_t errbit;     /* f26 */
  uint32_t regdbg;     /* w2d */
  uint32_t busy;       /* f30 */
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
  uint32_t bits;       /* s84 */
  uint32_t dumpo[8];   /* s88_8f */
  uint32_t dumpb[10];  /* s99_90 */
  uint32_t dumpk;      /* s9a */
  uint32_t dumpi;      /* s9b */
  uint32_t idump;      /* w9c */
  uint32_t vio;        /* s9e */
  uint32_t selila;     /* w9f */
};

typedef struct ft2d ft2d_t;


#if 0
void
regs_ft2d(ftsw_t* ftsw, struct timeval* tvp, fast_t* f, slow_t* s)
{
  int i;

  gettimeofday(tvp, 0);

  s->ftswid     = read_ftsw(ftsw, FT2D_FTSWID);
  s->cpldver    = read_ftsw(ftsw, FT2D_CPLDVER);
  s->conf       = read_ftsw(ftsw, FT2D_CONF);
  s->clksel     = read_ftsw(ftsw, FT2D_CLKSEL);
  s->fpgaid     = read_ftsw(ftsw, FT2D_FPGAID);
  s->fpgaver    = read_ftsw(ftsw, FT2D_FPGAVER);
  f->utime      = read_ftsw(ftsw, FT2D_UTIME);
  f->ctime      = read_ftsw(ftsw, FT2D_CTIME);
  s->exprun     = read_ftsw(ftsw, FT2D_EXPRUN);
  s->dmask      = read_ftsw(ftsw, FT2D_DMASK);
  s->addr       = read_ftsw(ftsw, FT2D_ADDR);
  s->suser      = read_ftsw(ftsw, FT2D_SUSER);
  s->jpd        = read_ftsw(ftsw, FT2D_JPD);
  s->jpll       = read_ftsw(ftsw, FT2D_JPLL);
  f->ttup       = read_ftsw(ftsw, FT2D_TTUP);
  f->trgtag     = read_ftsw(ftsw, FT2D_TRGTAG);
  f->trgtyp     = read_ftsw(ftsw, FT2D_TRGTYP);
  s->errutim    = read_ftsw(ftsw, FT2D_ERRUTIM);
  s->errctim    = read_ftsw(ftsw, FT2D_ERRCTIM);
  f->errport    = read_ftsw(ftsw, FT2D_ERRPORT);
  f->errbit     = read_ftsw(ftsw, FT2D_ERRBIT);
  s->regdbg     = read_ftsw(ftsw, FT2D_REGDBG);
  f->busy       = read_ftsw(ftsw, FT2D_BUSY);
  f->obsyin     = read_ftsw(ftsw, FT2D_OBSYIN);
  f->oerrin     = read_ftsw(ftsw, FT2D_OERRIN);
  f->olinkdn    = read_ftsw(ftsw, FT2D_OLINKDN);
  f->oalive     = read_ftsw(ftsw, FT2D_OALIVE);
  f->olinkup    = read_ftsw(ftsw, FT2D_OLINKUP);
  f->b2ldn      = read_ftsw(ftsw, FT2D_B2LDN);
  f->plldn      = read_ftsw(ftsw, FT2D_PLLDN);
  for (i = 0; i <= 9; i++) {
    f->odata[i]   = read_ftsw(ftsw, FT2D_ODATA + i * 4);
  };
  for (i = 0; i <= 9; i++) {
    f->odatb[i]   = read_ftsw(ftsw, FT2D_ODATB + i * 4);
  };
  for (i = 0; i <= 9; i++) {
    f->odatc[i]   = read_ftsw(ftsw, FT2D_ODATC + i * 4);
  };
  s->omanual    = read_ftsw(ftsw, FT2D_OMANUAL);
  s->clrdelay   = read_ftsw(ftsw, FT2D_CLRDELAY);
  s->incdelay   = read_ftsw(ftsw, FT2D_INCDELAY);
  s->bits       = read_ftsw(ftsw, FT2D_BITS);
  for (i = 0; i <= 7; i++) {
    s->dumpo[i]   = read_ftsw(ftsw, FT2D_DUMPO + i * 4);
  };
  for (i = 0; i <= 9; i++) {
    s->dumpb[i]   = read_ftsw(ftsw, FT2D_DUMPB + i * 4);
  };
  s->dumpk      = read_ftsw(ftsw, FT2D_DUMPK);
  s->dumpi      = read_ftsw(ftsw, FT2D_DUMPI);
  s->idump      = read_ftsw(ftsw, FT2D_IDUMP);
  s->vio        = read_ftsw(ftsw, FT2D_VIO);
  s->selila     = read_ftsw(ftsw, FT2D_SELILA);
}
#endif /* 0 */
