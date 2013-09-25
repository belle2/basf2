#ifndef _ftsw_h_
#define _ftsw_h_

/* ---------------------------------------------------------------------- *\

   ftsw.h

   Definitions for the FTSW registers and a few utility functions

   Mikihiko Nakao, KEK IPNS



   ------------------------
   FTSW register definition
   ------------------------

\* ---------------------------------------------------------------------- */

/* register map */
#define FTSW_SID     (0x0000 >> 2)
#define FTSW_SVER    (0x0010 >> 2)
#define FTSW_CONF    (0x0020 >> 2)
#define FTSW_CCLK    (0x0030 >> 2)
#define FTSW_CLKSEL  (0x0040 >> 2)
#define FTSW_CLKCNT  (0x0050 >> 2)
#define FTSW_CLKMON  (0x0060 >> 2)
#define FTSW_CLKSTA  (0x0070 >> 2)
#define FTSW_RST     (0x0080 >> 2)
/* undefined 0x0090..0x00f0 */
#define FTSW_VID     (0x0100 >> 2)
#define FTSW_VVER    (0x0110 >> 2)
#define FTSW_JSET    (0x01c0 >> 2)
#define FTSW_JGET    (0x01d0 >> 2)
#define FTSW_JPD     (0x01e0 >> 2)
#define FTSW_JSTA    (0x01f0 >> 2)

/* open mode */
#define FTSW_RDONLY    0x00
#define FTSW_RDWR      0x01
#define FTSW_USEFIFO   0x02

#define FTSW_VMEBASE  0x03000000
#define FTSW_VMEUNIT  0x00010000
#define FTSW_MMAPSIZE 0x00010000
#define SPARC_VMEDEV  "/dev/vme32d32"

#ifdef USE_LINUX_VME_UNIVERSE
#include <vme/vme.h>
#include <vme/vme_api.h>
#endif

#define M012_SERIAL      7
#define M012_SELECTMAP   6

typedef struct {
  int id;        /* n-th tt-io, starting from 0 */
  int writeflag; /* 0=readonly, 1=readwrite */
  int fd;        /* for device driver */
  int fdfifo;    /* for device driver */
  volatile unsigned* mapadrs;  /* for memory map */
#ifdef USE_LINUX_VME_UNIVERSE
  vme_bus_handle_t bus_handle;
  vme_master_handle_t window_handle;
#endif
} ftsw_t;

/* library functions */
#ifdef __cplusplus
extern "C" {
#endif
ftsw_t* ftsw_open(int ftswid, int flag);
int ftsw_close(ftsw_t*);
int ftsw_read(ftsw_t*, int regid);
int ftsw_readfifo(ftsw_t*, unsigned* bufp);
int ftsw_write(ftsw_t*, int regid, int value);
void ftsw_debug(int value);

void ftsw_write_fpga(ftsw_t* ftsw, int m012, int ch, int n);
int ftsw_boot_fpga(ftsw_t* ftsw, const char* file, int verbose, int forced, int m012);
int ftsw_trigger_single(ftsw_t* ftsw);
int ftsw_stop_trigger(ftsw_t* ftsw);


#ifdef __cplusplus
}
#endif

#endif
