
#ifndef __tt4r_h__
#define __tt4r_h__

const int tt4r_revision = 1;

#define TT4R_CPLDID  (0x000 >> 2)
#define TT4R_CPLDVER (0x010 >> 2)
#define TT4R_PRGM    (0x020 >> 2)
#define TT4R_SCLK    (0x030 >> 2)
#define TT4R_SER1    (0x0d0 >> 2)
#define TT4R_SER2    (0x0e0 >> 2)
#define TT4R_SER3    (0x0f0 >> 2)

#define TT4R_ID      (0x100 >> 2)
#define TT4R_VER     (0x110 >> 2)
#define TT4R_CSR     (0x120 >> 2)
#define TT4R_CSR2    (0x130 >> 2)
#define TT4R_UIO     (0x140 >> 2)
#define TT4R_FIN     (0x150 >> 2)
#define TT4R_UIN     (0x160 >> 2)
#define TT4R_FSTA    (0x170 >> 2)
#define TT4R_URAW    (0x180 >> 2)
#define TT4R_UTAG    (0x190 >> 2)
#define TT4R_REVO    (0x1a0 >> 2)
#define TT4R_NIBB    (0x1b0 >> 2)
#define TT4R_NCLK    (0x1c0 >> 2)

#define TT4R_DUMTRG  (0x200 >> 2)
#define TT4R_TLAST   (0x210 >> 2)
#define TT4R_RATE    (0x220 >> 2)
#define TT4R_DTAG    (0x230 >> 2)
#define TT4R_DTAGR   (0x240 >> 2)
#define TT4R_DEPTH   (0x250 >> 2)

#define TT4R_FIFO0   (0x300 >> 2)
#define TT4R_FIFO1   (0x314 >> 2) /* to be compatible with both v3 and v4 */

#define TT4R_DUMMEM  (0x320 >> 2)
#define TT4R_DUMSTA  (0x330 >> 2)

#include <stdint.h>

struct tt4r {
  uint8_t cpldid;  /* 00 & 0xf */
  uint8_t cpldver; /* 01 & 0xf */
  uint8_t prgm;    /* 02 & 0xf */
  uint8_t sclk;    /* 03 & 0xf */
  uint16_t flag;   /* space filler */
  uint16_t serial; /* (0f & 0xf)|((0e & 0xf)<<4)|((0d & 0xf)<<8) */
  uint32_t id;     /* 10 */
  uint32_t ver;    /* 11 */
  uint32_t csr;    /* 12 */
  uint32_t csr2;   /* 13 */
  uint32_t uio;    /* 14 */
  uint32_t fin;    /* 15 */
  uint32_t uin;    /* 16 */
  uint32_t fsta;   /* 17 */
  uint32_t uraw;   /* 18 */
  uint32_t utag;   /* 19 */
  uint32_t revo;   /* 1a */
  uint32_t nibb;   /* 1b */
  uint32_t nclk;   /* 1c */
  /* nothing in 1d..1f */
  uint32_t dumtrg; /* 20 */
  uint32_t tlast;  /* 21 */
  uint32_t rate;   /* 22 */
  uint32_t dtag;   /* 23 */
  uint32_t dtagr;  /* 24 */
  uint32_t depth;  /* 25 */
  /* should-not-be-here: uint32_t fifo0; */ /* 30 */
  /* should-not-be-here: uint32_t fifo1; */ /* 31 */
  /* should-not-be-here: uint32_t dummem;*/ /* 32 */
  uint32_t dumsta; /* 33 */
};

typedef struct tt4r tt4r_t;

#if defined(__cplusplus)
extern "C" {
#endif
void readregs_4r19(ttrx_t* ttrx, tt4r_t* t);
void tt4r19_show(const tt4r_t* t);
#if defined(__cplusplus)
}
#endif

#endif /* __tt4r_h__ */
