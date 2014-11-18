/*
  hsreg.h

  HSLB registers

  0x00       -- serial write register
  0x01..0x66 -- user defined registers (all used in the CDC example)
  0x67       -- user defined FEE control register
  0x68..0x6d -- Belle2link internal FEE registers including 32-bit extension
  0x6e..0x6f -- HSLB 32-bit extension
  0x70..0x7f -- HSLB and FINESSE registers

*/


#ifndef __HSREG_H__
#define __HSREG_H__

/* FEE registers as implemented in belle2link_cdc */

#define HSREG_SERIAL    0x00
#define HSREG_PDSTL     0x01 /* 0x01 .. 0x60 for 48 ch (x2) */
#define HSREG_PDSTLMIN  0x01
#define HSREG_PDSTLMAX  0x60
#define HSREG_ADCTH     0x61 /* same as lower 8 bit of ADC threshold setting */
#define HSREG_ADCL      0x61
#define HSREG_ADCH      0x62
#define HSREG_WINDOW    0x63
#define HSREG_DELAY     0x64
#define HSREG_TDCTH     0x65 /* same as lower 8 bit of DAC setting for TDC */
#define HSREG_DACL      0x65
#define HSREG_DACH      0x66
#define HSREG_CDCCONT   0x67 /* part of b2ldo (FEE_CONTROL) moved from 0x69 */

/* FEE registers for Belle2link internal use */

#define HSREG_CRCERR    0x68
#define HSREG_FEECONT   0x69
#define HSREG_D32A      0x6a
#define HSREG_D32B      0x6b
#define HSREG_D32C      0x6c
#define HSREG_D32D      0x6d

/* HSLB registers in addition to predefined FINESSE registers */

#define HSREG_D32       0x6e
#define HSREG_A32       0x6f
#define HSREG_HWVER     0x70
#define HSREG_FWVER     0x71
#define HSREG_STAT      0x72   /* b2ldo (STATUS) */
#define HSREG_CSR       0x73   /* b2ldo (CONTROL) */
#define HSREG_CCLK      0x74   /* cpld */
#define HSREG_CONF      0x75   /* cpld */
#define HSREG_CPLDVER   0x76   /* cpld */
#define HSREG_FEEHWTYPE 0x77
#define HSREG_FEESERIAL 0x78
#define HSREG_FEEFWTYPE 0x79
#define HSREG_FEEFWVER  0x7a

#define B2LFEE_UNDEF   0
#define B2LFEE_SVD     1
#define B2LFEE_CDC     2
#define B2LFEE_BPID    3
#define B2LFEE_EPID    4
#define B2LFEE_ECL     5
#define B2LFEE_KLM     6
#define B2LFEE_TRG     7
#define B2LFEE_TEST    15

/* FINESSE generic registers */

#define HSREG_SERIAL1  0x7b   /* cpld lower  8-bit */
#define HSREG_SERIAL2  0x7c   /* cpld higher 8-bit */
#define HSREG_FINTYPE1 0x7d   /* cpld lower  8-bit */
#define HSREG_FINTYPE2 0x7e   /* cpld higher 8-bit */


/* HSLB 32-bit register extension */

#define HSREGL_ID      0x80
#define HSREGL_VER     0x81
#define HSREGL_RESET   0x82
#define HSREGL_STAT    0x83
/*
  stal(16#83#)(0) <= not led1;  -- not LinkOn
  stal(16#83#)(1) <= led2;  -- Trans
  stal(16#83#)(2) <= led3;  -- FFUL
  stal(16#83#)(3) <= led4;  -- LinkError
  stal(16#83#)(4) <= led5;  -- NWFF
  stal(16#83#)(5) <= led6;  -- sta_rxbyteisaligned
  stal(16#83#)(6) <= not fwenb;
  stal(16#83#)(31 downto 28) <= cnt_oflow;
  stal(16#83#)(27 downto 24) <= cnt_badalign;
  stal(16#83#)(23 downto 20) <= cnt_realign;
  stal(16#83#)(18) <= not rxaligned;
  stal(16#83#)(17) <= rxrealign;
  stal(16#83#)(16) <= badalign;
  stal(16#83#)(15 downto 12) <= state;
*/
#define HSREGL_RXDATA  0x84  /* stal(16#84#) <= cntcrcerr & rxdata; */

/* 0 parameter */
#define HSDO_NOP      0x100
#define HSDO_LINK     0x101
#define HSDO_CHECKFEE 0x102
#define HSDO_TRGOFF   0x104
#define HSDO_TRGON    0x105

/* 0 parameter (obsolete, unused) */
#define HSDO_TTTRG    0x106
#define HSDO_DUMTRG   0x107

/* 0 parameter (CDC special) */
#define HSDO_SUPMODE  0x108
#define HSDO_RAWMODE  0x109

/* 1 parameter */
#define HSDO_STREAM   0x200

/* 1 parameter (CDC special) */
#define HSDO_PDSTLALL 0x201
#define HSDO_ADCTH    0x202
#define HSDO_TDCTH    0x203
#define HSDO_DELAY    0x204
#define HSDO_WINDOW   0x205

/* 2 parameters */
#define HSDO_FEE8     0x300
#define HSDO_FEE32    0x301

/* 2 parameters (CDC special) */
#define HSDO_PDSTL    0x302

#endif /* __HSREG_H__ */

