/* ---------------------------------------------------------------------- *\

   ttrx.h

   Definitions for the TT-RX registers and a few utility functions

   Mikihiko Nakao, KEK IPNS

   version
   2003110400  working version
   2004083100  TT-RX v4
   2004090700  TT-RX v4 (FPGA v0.01)
   2005050800  unified registers
   2005081600  serial number
   2005081601  extern "C"

\* ---------------------------------------------------------------------- */

#ifndef __ttrx_h__
#define __ttrx_h__

/* register map */
#define TTRX_PID     (0x000 >> 2)
#define TTRX_PVER    (0x010 >> 2)
#define TTRX_PRGM    (0x020 >> 2)
#define TTRX_SCLK    (0x030 >> 2)
#define TTRX_SER1    (0x0d0 >> 2)
#define TTRX_SER2    (0x0e0 >> 2)
#define TTRX_SER3    (0x0f0 >> 2)

#define TTRX_ID      (0x100 >> 2)
#define TTRX_VER     (0x110 >> 2)
#define TTRX_CSR     (0x120 >> 2)
#define TTRX_CLK     (0x130 >> 2)
#define TTRX_INTR    (0x140 >> 2)
#define TTRX_MASK    (0x150 >> 2)

#define TTRX_TRIG    (0x200 >> 2)
#define TTRX_TLAST   (0x210 >> 2)
#define TTRX_RATE    (0x220 >> 2)
#define TTRX_TAG     (0x230 >> 2)
#define TTRX_TAGR    (0x240 >> 2)
#define TTRX_DEPTH   (0x250 >> 2)

#define TTRX_FIFO1   (0x300 >> 2)
#define TTRX_FIFO2   (0x314 >> 2) /* to be compatible with both v3 and v4 */

#define TTRX_USTAT   (0x400 >> 2)
#define TTRX_UERRS   (0x410 >> 2)
#define TTRX_UCMD    (0x420 >> 2)
#define TTRX_UCHK    (0x430 >> 2)
#define TTRX_URAW    (0x440 >> 2)

#define TTRX_EXT     (0x600 >> 2)

#define TTRX_RDONLY    0x00
#define TTRX_RDWR      0x01
#define TTRX_USEFIFO   0x02
#define TTRX_USERMODE  0x00
#define TTRX_MMAPMODE  0x10

#define TTRX_MMAPSIZE 4096
#define TTRX_DEV_NAME  "/dev/ttrx"
#define TTRX_DEVFIFO_NAME  "/dev/ttrx_fifo"

typedef struct {
  int id;        /* n-th tt-rx, starting from 0 */
  int writeflag; /* 0=readonly, 1=readwrite */
  int fd;        /* for device driver */
  int fdfifo;    /* for device driver (fifo device) */
  volatile unsigned* mapadrs;  /* for memory map */
  int prev;      /* to insert wait between read and write for memory map */
} ttrx_t;

#if defined(__cplusplus)
extern "C" {
#endif
/* library functions */
ttrx_t* open_ttrx(int ttrxid, int flag);
int close_ttrx(ttrx_t*);
int read_ttrx(ttrx_t*, int regid);
int write_ttrx(ttrx_t*, int regid, int value);
int readfifo_ttrx(ttrx_t*, unsigned* fifo);
ttrx_t* init_ttrx();
int start_ttrx(ttrx_t*);
int start_ttrx_nofifo(ttrx_t*);
int stop_ttrx(ttrx_t*);
char* getversion_ttrx(ttrx_t*);
#if defined(__cplusplus)
  int trgdelay_ttrx(ttrx_t*, int delay_in_sclk_unit = -1);
}
#else
  int trgdelay_ttrx(ttrx_t*, int delay_in_sclk_unit);
#endif
#endif /* __ttrx_h__ */
