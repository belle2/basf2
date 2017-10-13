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

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif
#if defined(__dummy_close_bracket_to_cheat_emacs_auto_indent)
}
#endif

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
  -- input
  stal(16#81#)    <= x"0000" & std_logic_vector(to_unsigned(VERSION, 16));

  stal(16#83#)(0) <= not linkup;
  stal(16#83#)(1) <= disable;
  stal(16#83#)(2) <= led3;  -- FFUL
  stal(16#83#)(3) <= led4;  -- LinkError
  stal(16#83#)(4) <= led5;  -- NWFF
  stal(16#83#)(5) <= sta_bad127;
  stal(16#83#)(6) <= not plllk;
  stal(16#83#)(7) <= not stapll2;
  stal(16#83#)(11 downto  8) <= stateff;
  stal(16#83#)(15 downto 12) <= staterx;
  stal(16#83#)(19 downto 16) <= statepr;
  stal(16#83#)(23 downto 20) <= statept;
  stal(16#83#)(28 downto 24) <= statetx;
  stal(16#83#)(29) <= not stapll;
  stal(16#83#)(30) <= not fwenb;
  stal(16#83#)(31) <= stalinkdown;

  stal(16#84#) <= cntlinkdown(7 downto 0) & cntcrcerr(7 downto 0) & rxdata;
  stal(16#85#) <= cnt_evtsz & cnt_maxsz;
  stal(16#86#) <= cnt_event;  -- number of events into copper fifo
  stal(16#87#) <= cnt_sec127;

  stal(16#88#) <= cnt_nword(39 downto 8);

  stal(16#89#)(31 downto 28) <= cnt_oflow;
  stal(16#89#)(27 downto 24) <= cnt_badalign;
  stal(16#89#)(23 downto 20) <= cnt_realign;
  stal(16#89#)(19) <= badalign;
  stal(16#89#)(18) <= rxrealign;
  stal(16#89#)(17) <= not rxaligned;
  stal(16#89#)(16) <= not led6;  -- sta_rxbyteisaligned
  stal(16#89#)(15 downto  8) <= cnt_fwclk;
  stal(16#89#)(7  downto  0) <= cnt_nword(7 downto 0);

  stal(16#8c#)(31 downto 16) <= cntvetoidle;
  stal(16#8c#)(15 downto 0)  <= cntvetodata;
  stal(16#8d#)(25 downto 24) <= bufvetodown(53 downto 52);
  stal(16#8d#)(21 downto 20) <= bufvetodown(51 downto 50);
  stal(16#8d#)(17 downto 16) <= bufvetodown(49 downto 48);
  stal(16#8d#)(15 downto 0)  <= bufvetodown(47 downto 32);
  stal(16#8e#)(31 downto 0)  <= bufvetodown(31 downto  0);

  stal(16#8f#) <= dbg;

  -- output
  setrunreset  <= regl(16#82#)(0);
  b2lreset     <= regl(16#82#)(4);
  txreset      <= regl(16#82#)(8);
  gtpreset     <= regl(16#82#)(12);
  clrpll       <= regl(16#82#)(16);
  clrpll2      <= regl(16#82#)(20);
  usebadidle   <= regl(16#8b#)(1);
  usebaddata   <= regl(16#8b#)(0);
*/
#define HSREGL_RXDATA  0x84  /* stal(16#84#) <= cntcrcerr & rxdata; */
#define HSREGL_EVENTSZ 0x85
#define HSREGL_NEVENT  0x86
#define HSREGL_UPTIME  0x87
#define HSREGL_NKBYTE  0x88
#define HSREGL_NWORD   0x89
#define HSREGL_VETOSET 0x8b
#define HSREGL_VETOCNT 0x8c
#define HSREGL_VETOBUF0 0x8d
#define HSREGL_VETOBUF1 0x8e
#define HSREGL_CNTFF00     0x95 /* new in hslb055 */
#define HSREGL_CNTDATAPKT  0x96 /* new in hslb055 */
#define HSREGL_CNTDATAWORD 0x97 /* new in hslb055 */

static const int hsreg_revision = 1;

struct hsreg {
  uint8_t  stat;    /* 72 */
  uint8_t  csr;     /* 73 */
  uint8_t  xbusy;   /* 74 */
  uint8_t  conf;    /* 75 */
  uint8_t  fincpld; /* 76 */
  uint8_t  feehw;   /* 77 >> 4 */
  uint8_t  feefw;   /* 79 >> 4 */
  uint8_t  feecrce; /* 68 */
  uint16_t feeser;  /* 78 | ((77 & f) << 8) */
  uint16_t feever;  /* 7a | ((79 & f) << 8) */
  uint16_t finser;  /* (7c << 8) | 7b */
  uint16_t fintyp;  /* (7e << 8) | 7d */
  uint32_t hslbid;  /* 80 */
  uint32_t hslbver; /* 81 */
  uint32_t hslbrst; /* 82 */
  uint32_t hslbsta; /* 83 */
  uint32_t rxdata;  /* 84 */
  uint32_t eventsz; /* 85 */
  uint32_t nevent;  /* 86 */
  uint32_t uptime;  /* 87 */
  uint32_t nkbyte;  /* 88 */
  uint32_t nword;   /* 89 */
  uint32_t vetoset; /* 8b */
  uint32_t vetocnt; /* 8c */
  uint32_t vetobuf[2]; /* 8d, 8e */
  uint32_t cntff00;     /* 95 */
  uint32_t cntdatapkt;  /* 96 */
  uint32_t cntdataword; /* 97 */
};

typedef struct hsreg hsreg_t;

int hsreg_read(int fd /* 0-3 */, hsreg_t*);
int hsreg_getfee(int fd /* 0-3 */, hsreg_t*);

#if defined(__dummy_open_bracket_to_cheat_emacs_auto_indent)
__dummy_open_bracket_to_cheat_emacs_auto_indent {
#endif
#if defined(__cplusplus)
}
#endif

#endif /* __HSREG_H__ */

