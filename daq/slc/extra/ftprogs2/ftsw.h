/* ---------------------------------------------------------------------- *\

   ftsw.h

   Definitions for the FTSW registers and a few utility functions

   Mikihiko Nakao, KEK IPNS

   20130927 updated for ft2u023 / pocket_ttd
   20131207 missing CLKSEL was added

   ------------------------
   FTSW register definition
   ------------------------

   00 FTSWID (RW)
      initial value: 0x46545357 (ASCII code for "FTSW"),
      can be changed to anything else, but not recommended to change

   01 CPLDVER (R)
      [31:16] (16-bit) Board ID
      [15:0]  (16-bit) CPLD firmware version

   02 CONF (RW)
      write 0x41 to disable configuration flash memory
      write 0x87 to assert PROGRAM* of FPGA
      write 0x86 to deassert PROGRAM* of FPGA
      write (0x08 + m012) to set M[2:0] of FPGA to m012
      read
        [7] DONE of FPGA (program is done and FPGA is ready)
  [6] not-busy (FPGA is not busy for programming)
  [5] always '0'
  [4] program is controlled by the user program
  [3] INIT* of FPGA
  [2:0] M[2:0] of FPGA, i.e., 7 for serial-slave and 6 for
        selectMAP-slave (parallel)

   03 CCLK (W)
      write generates CCLK to FPGA with lowest 8-bit (or 1-bit) for
      programming in selectMAP-slave (serial-slave) mode

   04 CLKSEL (RW)
      [0:1] (2-bit) 1:iclk, 2:fmc, 0 or 3:xtal

   08 SYSRST (W)
      write 1 to generate SYSRESET* of VME

   10 FPGAID (RW)
      initial value: ASCII code of FTSW FPGA firmware name, e.g.,
      0x46543255 for FT2U firmware, can be changed to anything else,
      but not recommended to change

   11 FPGAVER (R)
      [31:16] (16-bit) board ID
      [15:0]  (16-bit) FPGA firmware version

      if the board ID of FPGAVER and CPLDVER do not agree, FPGA is not
      properly receiving the board ID from CPLD

   12 SETTIME (RW)
      [31:0] (32-bit) unix-time, i.e., time as the number of seconds
                      since the Epoch, 1970-01-01 00:00:00 +0000 (UTC),
                      as the reference for the time of the system, and
                      also to record when the time was set

   13 CLKFREQ (RW)
      [23:0] (24-bit) initial value: 0x0952980, lowest 24-bit of
             0x7952980 = 127216000 to define the clock frequency to
       increment utime counted in the unit of second

   14 UTIME (R(W))
      [31:0] (32-bit) current unix-time counted by FTSW

      Writing any value to this register will hold the value of UTIME,
      CTIME, UDEAD, CDEAD, TINCNT and TOUTCNT for one second, to
      properly calculate the trigger rate, dead time, etc.  Read CTIME
      as the last register and checking bit-31 to be '1' to ensure
      the all register are held.

   15 CTIME (R)
      [26:0] (27-bit) current time within the second in the system
                      clock unit, counted by FTSW
      [31]   (1-bit)  UTIME, CTIME, UDEAD, CDEAD, TINCNT, TOUTCNT are held

   20 RESET ((R)/W)
      [31]   nofifo:   no FIFO full to block the trigger
      [30]   autorst:  auto reset mode (runreset when connection changes)
      [28]   genbor:   generate begin-of-run trigger at run start
      [24]   usetlu:   connect to TLU at AUX port
      [16]   regbusy:  artificial busy by software
      [12]   clrictrl: reset IDELAYCTRL
      [9]    feereset: FEE reset  (write only)
      [8]    b2lreset: Belle2link reset  (write only)
      [4]    cntreset: FTSW counter reset (write only)
      [0]    runreset: run reset (write only)

   28 TRGSET (RW)
      write anything: reset dummy trigger cycle
      [31:20] (12-bit) trgopt: optional parameter for dummy trigger
      [17:8]  (10-bit) rateval: dummy trigger rate, linear part
      [7:4]   (4-bit)  rateexp: dummy trigger rate, exponent part
      [3]     (1-bit)  (unused)
      [2:0]   (3-bit)  seltrg: type of trigger source
                        0: none
      1: IN 1-2 pair
      2: AUX 7-8 pair
      3: TLU trigger
      4: pulse dummy trigger (uniform over revolution)
      5: revo dummy trigger (fixed phase in revolution)
      6: random dummy trigger (uniform random interval)
      7: poisson dummy trigger (poisson interval)
      See trigft.c for more detail of dummy trigger setting.

   2d STAFIFO (R)
      [31]    (1-bit) fifoful:  FIFO is full
      [30]    (1-bit) fifoorun: FIFO overrun
      [28]    (1-bit) fifoemp:  FIFO is empty
      [25:24] (2-bit) fifoahi:  n-th 32-bit is being read
      [0]     (1-bit) trgen:    trigger is enabled

   2e FIFO (R)
      [31:0] (32-bit) fifo: four 32-bit words are sequentially read out,
          0xffffffff if empty
         word 0
    [31]    (1-bit)  0 if fifo data is valid
    [30:4]  (27-bit) trigger time in clock unit
    [3:0]   (4-bit)  trigger type
         word 1
    [31:0]  (32-bit) trigger time in second unit
         word 2
    [31:0]  (32-bit) trigger tag (incremented from 0)
         word 3
    [14:0]  (15-bit) TLU tag (incremented from 1)

*/

#ifndef __FTSW_H__
#define __FTSW_H__

/* register map */
#define FTSWREG(a)  ((a)<<2)
#define FTSWADDR(a) ((a)>>2)

#define FTSWREG_FTSWID   FTSWREG(0x00)  /* RW: "FTSW" identifier */
#define FTSWREG_CPLDVER  FTSWREG(0x01)  /* R:  CPLD firmware version */
#define FTSWREG_CONF     FTSWREG(0x02)  /* RW: for FPGA programming */
#define FTSWREG_CCLK     FTSWREG(0x03)  /* RW: for FPGA programming */
#define FTSWREG_CLKSEL   FTSWREG(0x04)  /* RW: clock select */
#define FTSWREG_SYSRST   FTSWREG(0x08)  /* W:  sysreset */

#define FTSWREG_FPGAID   FTSWREG(0x10)  /* RW: FPGA firmware type */
#define FTSWREG_FPGAVER  FTSWREG(0x11)  /* R:  FPGA firmware version */

#define FTSWREG_SETUTIM  FTSWREG(0x12)  /* RW: set utime */
#define FTSWREG_CLKFREQ  FTSWREG(0x13)  /* RW: # of clocks for 1 second */
#define FTSWREG_UTIME    FTSWREG(0x14)  /* R:  counted utime in sec */
#define FTSWREG_CTIME    FTSWREG(0x15)  /* R:  counted ctime in clock */
#define FTSWREG_JSTA     FTSWREG(0x1f)  /* R:  clock status */
#define FTSWREG_RESET    FTSWREG(0x20)  /* RW: run/count/b2link reset, etc */
#define FTSWREG_TRGSET   FTSWREG(0x28)  /* RW: source, rate, opt */
#define FTSWREG_TLIMIT   FTSWREG(0x29)  /* RW: trigger number limit */
#define FTSWREG_STAFIFO  FTSWREG(0x2d)  /* R:  FIFO status */
#define FTSWREG_FIFO     FTSWREG(0x2e)  /* R:  FIFO of trigger data */


/* open mode */
#define FTSW_RDONLY    0x00
#define FTSW_RDWR      0x01
#define FTSW_USEFIFO   0x02

#define FTSW_VMEBASE  0x02000000
#define FTSW_MAXID    0x100
#define FTSW_BASEID   0x100
#define FTSW_VMEUNIT  0x00010000
#define FTSW_MMAPSIZE 0x00010000
#define SPARC_VMEDEV  "/dev/vme32d32"

#ifdef USE_LINUX_VME_UNIVERSE
#include <vme/vme.h>
#include <vme/vme_api.h>
#endif /*  USE_LINUX_VME_UNIVERSE */

typedef struct {
  int id;        /* n-th tt-io, starting from 0 */
  int writeflag; /* 0=readonly, 1=readwrite */
  int fd;        /* for device driver */
  int fdfifo;    /* for device driver */
  volatile unsigned* mapadrs;  /* for memory map */
#ifdef USE_LINUX_VME_UNIVERSE
  vme_bus_handle_t bus_handle;
  vme_master_handle_t window_handle;
#endif /*  USE_LINUX_VME_UNIVERSE */
} ftsw_t;

/* library functions */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
ftsw_t* open_ftsw(int ftswid, int flag);
int close_ftsw(ftsw_t*);
int read_ftsw(ftsw_t*, int regid);
int readfifo_ftsw(ftsw_t*, unsigned* bufp);
int write_ftsw(ftsw_t*, int regid, int value);
void debug_ftsw(int value);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FTSW_H__ */
