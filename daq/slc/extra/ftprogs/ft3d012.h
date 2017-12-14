/*

  ft3d012.h for ft[23][dr]012 and on
  (register map is changed from ft3[dr]011)

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
  -2f_2e
  f30 busy bsyin26 errin25 clkerr24 obsyin9.0
  f31 obsyin9.0
  f32 oerrin9.0
  f33 olinkdn9.0
  f34 oalive9.0
  f35 olinkup9.0
  f36 b2ldn9.0
  f37 plldn9.0
  f41_38 odata addr31.12 fee11 ttdn10 b2ldn9 tage8 fifoe7 ful6 seu5.4
  -            b2lup3 plllk2 ttup1 alive0
  -4b_42
  f55_4c odatb busy31 diff29.28 b2ltag27.16 b2lwe15.0
  -5f_56
  f69_60 odatc
  -7b_6a
  w7c omanual9.0
  w7d clrdelay9.0
  w7e incdelay9.0
  -7f
  s87_80 dumpo
  s88 dumpk
  s89 dumpi
  s8a oackq12.0 cntpacket31.28 bitb27.26 subb25.24 disp23.16
  -8f_8b
  s99_90 dumpb
  -9e_9a
  w9f selila11.8 oaux1.0
  end regs

*/

static const int ft3d_revision = 2;

#include <stdint.h>

/* below this line is output of ftregs.pl */

#ifndef FT3D_NUMO /* ft2r uses NUMO=12 */
#define FT3D_NUMO 10
#endif

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
#define FT3D_BUSY       FTSWREG(0x30)
#define FT3D_OBSYIN     FTSWREG(0x31)
#define FT3D_OERRIN     FTSWREG(0x32)
#define FT3D_OLINKDN    FTSWREG(0x33)
#define FT3D_OALIVE     FTSWREG(0x34)
#define FT3D_OLINKUP    FTSWREG(0x35)
#define FT3D_B2LDN      FTSWREG(0x36)
#define FT3D_PLLDN      FTSWREG(0x37)
#define FT3D_ODATA      FTSWREG(0x38)
#define FT3D_ODATA_MAX  FTSWREG(0x38 + FT3D_NUMO - 1)
#define FT3D_ODATB      FTSWREG(0x4c)
#define FT3D_ODATB_MAX  FTSWREG(0x4c + FT3D_NUMO - 1)
#define FT3D_ODATC      FTSWREG(0x60)
#define FT3D_ODATC_MAX  FTSWREG(0x60 + FT3D_NUMO - 1)
#define FT3D_OMANUAL    FTSWREG(0x7c)
#define FT3D_CLRDELAY   FTSWREG(0x7c)
#define FT3D_INCDELAY   FTSWREG(0x7c)
#define FT3D_DUMPO      FTSWREG(0x80)
#define FT3D_DUMPO_7    FTSWREG(0x87)
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
  uint32_t busy;       /* f30 */
  uint32_t obsyin;     /* f31 */
  uint32_t oerrin;     /* f32 */
  uint32_t olinkdn;    /* f33 */
  uint32_t oalive;     /* f34 */
  uint32_t olinkup;    /* f35 */
  uint32_t b2ldn;      /* f36 */
  uint32_t plldn;      /* f37 */
  uint32_t odata[FT3D_NUMO];  /* f41_38 */
  uint32_t odatb[FT3D_NUMO];  /* f4f_4c */
  uint32_t odatc[FT3D_NUMO];  /* f69_60 */
  uint32_t omanual;    /* w7c */
  uint32_t dumpo[8];   /* s87_80 */
  uint32_t dumpk;      /* s88 */
  uint32_t dumpi;      /* s89 */
  uint32_t oackq;      /* s8a */
  uint32_t dumpb[10];  /* s99_90 */
  uint32_t selila;     /* w9f */
};

typedef struct ft3d ft3d_t;
