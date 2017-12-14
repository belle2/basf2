/* ---------------------------------------------------------------------- *\

   ftsw.h

   Definitions for the FTSW registers and a few utility functions

   Mikihiko Nakao, KEK IPNS



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
                      since th Epoch, 1970-01-01 00:00:00 +0000 (UTC),
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

   16 UDEAD (R)
      [31:0] (32-bit) total deadtime in second unit since counter or run reset.

   17 CDEAD (R)
      [26:0] (27-bit) total deadtime in clock unit since counter or run reset.

   18 OMASK (RW)
      [31] S3 query
      [30:28] 0: to select all COPPER, 1-4: to select one COPPER
      [23:16] negate the clock output of OTRG mask
      [15:12] COPPER mask (bit '1' to disable each COPPER)
      [11:8]  0: to select all OTRG, 1-8: to select one OTRG
      [7:0]   OTRG mask (bit '1' to disable each OTRG)

   19 STAMISC (R)
      [25:16] S3 misc status (detail to be described later)
      [7:0]   link status summary (to be defined)

   1a JTAG (RW)
      JTAG port control and generating JTAG signals.  To be implemented later.

   1b JTDO (R)
      Read back TDO status.  To be implemented later

   1c JCTL (RW)
      Jitter cleaner control.
      Writing 0x0000000e to JCTL will show up the register 0 at JREG
      Writing 0x0000002e to JCTL will show up the register 1 at JREG
      Writing 0x0000004e to JCTL will show up the register 2 at JREG
      Do not write any other value

   1d JREG (R)
      Jitter cleaner control register.
      Register 0 should be 0x4a200d50
      Register 1 should be 0x03874061
      Register 2 may be 0x61003cb2, but some of the bits may be different

   1e JRST (RW)
      [31]  Power down jitter cleaner
      [1]   reset JSTA
      [0]   disable auto phase detection

   1f JSTA (RW)
      [31]             jitter cleaner PLL lock
      [30]             FPGA DCM lock
      [27:24] (4-bit)  jphase, should be 0xc
      [23:16] (8-bit)  jretry (number of retries to get the phase right)
      [15:0] (16-bit)  jcount should reach 0x8000 when stable

   20 RESET ((R)/W)
      [31]   nofifo:   no FIFO full to block the trigger
      [8]    b2lreset: Belle2link reset  (write only)
      [4]    cntreset: FTSW counter reset (write only)
      [0]    runreset: run reset (write only)

   21 UTIMRST (R)
      [31:0] (32-bit) time in second unit when counter or run reset.

   22 CTIMRST (R)
      [28]            reset source was run reset
      [27]            reset source was counter reset
      [26:0] (27-bit) time in clock unit when counter or run reset.

   23 UTIMERR (R)
      [31:0] (32-bit) time in second unit when an error occured.

   24 CTIMERR (R)
      [26:0] (27-bit) time in clock unit when an error occcured.

   25 ERRPORT (R)
      [11:0] (12-bit) bit mask of error location:
        [11]  error occured at COPPER port 4
         :                                 :
        [8]   error occured at COPPER port 1
        [7]   error occured at OTRG port 8
         :                               :
        [0]   error occured at OTRG port 1

   26 TLUCTRL (RW)
      (to be defined)

   27 TLUSTAT (R)
      [14:0] (15-bit) tlutag: trigger tag from TLU
      [31:24] (8-bit) cnt_tlurst: number of reset signal from TLU

   28 TRGSET (RW)
      write anything: start new trigger
      [31:20] (12-bit) trgopt: optional parameter for dummy trigger
      [17:8]  (10-bit) rateval: dummy trigger rate, linear part
      [7:4]   (4-bit)  rateexp: dummy trigger rate, exponent part
      [3]     (1-bit)  (unused)
      [2:0]   (3-bit)  seltrg: type of trigger source
                        0: none
      1: IN 1-2 pair
      2: AUX 7-8 pair
      3: rsv
      4: pulse dummy trigger (uniform over revolution)
      5: revo dummy trigger (fixed phase in revolution)
      6: random dummy trigger (uniform random interval)
      7: poisson dummy trigger (poisson interval)
      See trigft.c for more detail of dummy trigger setting.

   29 TLIMIT (RW)
      [31:0] (32-bit) number of triggers to generate
                      no trigger to generate if 0
          no limit to generate if 0xffffffff

   2a TINCNT (R)
      [31:0] (32-bit) number of trigger input since cnt/runreset

   2b TOUTCNT (R)
      [31:0] (32-bit) number of generated trigger since cnt/runreset

   2c TLAST (R)
      [31:0] (32-bit) number of triggers yet to generate

   2d STAFIFO (R)
      [31] (1-bit) fifoful:  FIFO is full
      [30] (1-bit) fifoorun: FIFO overrun
      [29] (1-bit) fifoahi:  0: higher 32-bit, 1: lower 32-bit
      [28] (1-bit) fifoemp:  FIFO is empty

   2e FIFO (R)
      [31:0] (32-bit) fifo: 64-bit FIFO data,
                            higher 32-bit at the first read,
                            lower 32-bit at the second read,
          0xffffffff if empty
         64-bit FIFO data is
    [63]    (1-bit)  0 if fifo data is valid
    [62:36] (27-bit) trigger time in clock unit
    [35:32] (4-bit)  trigger type
    [31:16] (16-bit) trigger time in second unit (lower 16-bit)
    [15:0]  (16-bit) trigger tag (incremented from 1)


   3c STA1A (R)
      status of OTRG port 1
      [31:12] addr
      [11]    feeerr
      [10]    tagerr or fifoerr
      [9]     seuerr
      [8]     busy
      [7:4]   tagdiff
      [3:0]   b2ldiff




\* ---------------------------------------------------------------------- */

#ifndef _FTSW_H_
#define _FTSW_H_

/* register map */
#define FTSWREG(a)  (a<<2)

#define FTSWREG_FTSWID   FTSWREG(0x00)  /* RW: "FTSW" identifier */
#define FTSWREG_CPLDVER  FTSWREG(0x01)  /* R:  CPLD firmware version */
#define FTSWREG_CONF     FTSWREG(0x02)  /* RW: for FPGA programming */
#define FTSWREG_CCLK     FTSWREG(0x03)  /* RW: for FPGA programming */

#define FTSWREG_FPGAID   FTSWREG(0x10)  /* RW: FPGA firmware type */
#define FTSWREG_FPGAVER  FTSWREG(0x11)  /* R:  FPGA firmware version */
#define FTSWREG_SETUTIM  FTSWREG(0x12)  /* RW: set utime */
#define FTSWREG_CLKFREQ  FTSWREG(0x13)  /* RW: # of clocks for 1 second */
#define FTSWREG_UTIME    FTSWREG(0x14)  /* R:  counted utime in sec */
#define FTSWREG_CTIME    FTSWREG(0x15)  /* R:  counted ctime in clock */
#define FTSWREG_UDEAD    FTSWREG(0x14)  /* R:  deadtime in sec */
#define FTSWREG_CDEAD    FTSWREG(0x15)  /* R:  deadtime in clock */
#define FTSWREG_OMASK    FTSWREG(0x18)  /* RW: select RJ-45 port */
#define FTSWREG_STAMISC  FTSWREG(0x19)  /* R:  link status of RJ-45 port */
#define FTSWREG_JTAG     FTSWREG(0x1a)  /* RW: JTAG control */
#define FTSWREG_JTDO     FTSWREG(0x1b)  /* R:  JTAG TDO */
#define FTSWREG_JCTL     FTSWREG(0x1c)  /* RW: jitter cleaner control */
#define FTSWREG_JREG     FTSWREG(0x1d)  /* R:  jitter cleaner register */
#define FTSWREG_JRST     FTSWREG(0x1e)  /* RW: jitter cleaner reset */
#define FTSWREG_JSTA     FTSWREG(0x1f)  /* R:  jitter cleaner status */

#define FTSWREG_RESET    FTSWREG(0x20)  /* RW: run/count/b2link reset, etc */
#define FTSWREG_UTIMRST  FTSWREG(0x21)  /* R:  when reset happened */
#define FTSWREG_CTMIRST  FTSWREG(0x22)  /* R:  when reset happened */
#define FTSWREG_UTIMERR  FTSWREG(0x23)  /* R:  when error happened */
#define FTSWREG_CTMIERR  FTSWREG(0x24)  /* R:  when error happened */
#define FTSWREG_ERRSRC   FTSWREG(0x25)  /* R:  where error happened */
#define FTSWREG_TLUCTRL  FTSWREG(0x26)  /* RW: TLU control */
#define FTSWREG_TLUSTAT  FTSWREG(0x27)  /* R:  TLU status */

#define FTSWREG_TRGSET   FTSWREG(0x28)  /* RW: source, rate, opt */
#define FTSWREG_TLIMIT   FTSWREG(0x29)  /* RW: # of trigger to generate */
#define FTSWREG_TINCNT   FTSWREG(0x2a)  /* R:  # of trigger-in since reset */
#define FTSWREG_TOUTCNT  FTSWREG(0x2b)  /* R:  # of trigger-out since reset */
#define FTSWREG_TLAST    FTSWREG(0x2c)  /* R:  # of trigger yet to generate */
#define FTSWREG_STAFIFO  FTSWREG(0x2d)  /* R:  FIFO status */
#define FTSWREG_FIFO     FTSWREG(0x2e)  /* R:  FIFO of trigger data */
/* FTSWREG(0x2f) is not used yet */

#define FTSWREG_ENSTAT   FTSWREG(0x30)  /* R:  encoder status */
#define FTSWREG_REVOPOS  FTSWREG(0x31)  /* RW: revosig position  */
#define FTSWREG_REVOIN   FTSWREG(0x32)  /* R:  revosig input status */
#define FTSWREG_REVOCNT  FTSWREG(0x33)  /* R:  bad revolution count */
#define FTSWREG_SETADDR  FTSWREG(0x34)  /* RW: set address for command */
#define FTSWREG_SETCMD   FTSWREG(0x35)  /* RW: command */
/* FTSWREG(0x36) is not used yet */
/* FTSWREG(0x37) is not used yet */

#define FTSWREG_STAT     FTSWREG(0x38)  /* R:  output port status summary */
#define FTSWREG_LINKUP   FTSWREG(0x39)  /* R:  output port link status  */
#define FTSWREG_ERROR    FTSWREG(0x38)  /* R:  output port error summary */
#define FTSWREG_ACKSIG   FTSWREG(0x3b)  /* R:  raw ACK signal */
#define FTSWREG_STA0A    FTSWREG(0x3c)  /* R:  status for port 1 */
#define FTSWREG_STA0B    FTSWREG(0x3d)  /* R:  status for port 1 */
#define FTSWREG_STA1A    FTSWREG(0x3e)  /* R:  status for port 2 */
#define FTSWREG_STA1B    FTSWREG(0x3f)  /* R:  status for port 2 */
#define FTSWREG_STA2A    FTSWREG(0x40)  /* R:  status for port 3 */
#define FTSWREG_STA2B    FTSWREG(0x41)  /* R:  status for port 3 */
#define FTSWREG_STA3A    FTSWREG(0x42)  /* R:  status for port 4 */
#define FTSWREG_STA3B    FTSWREG(0x43)  /* R:  status for port 4 */
#define FTSWREG_STA4A    FTSWREG(0x44)  /* R:  status for port 5 */
#define FTSWREG_STA4B    FTSWREG(0x45)  /* R:  status for port 5 */
#define FTSWREG_STA5A    FTSWREG(0x46)  /* R:  status for port 6 */
#define FTSWREG_STA5B    FTSWREG(0x47)  /* R:  status for port 6 */
#define FTSWREG_STA6A    FTSWREG(0x48)  /* R:  status for port 7 */
#define FTSWREG_STA6B    FTSWREG(0x49)  /* R:  status for port 7 */
#define FTSWREG_STA7A    FTSWREG(0x4a)  /* R:  status for port 8 */
#define FTSWREG_STA7B    FTSWREG(0x4b)  /* R:  status for port 8 */
#define FTSWREG_STA8A    FTSWREG(0x4c)  /* R:  status for port 9 */
#define FTSWREG_STA8B    FTSWREG(0x4d)  /* R:  status for port 9 */
#define FTSWREG_STA9A    FTSWREG(0x4e)  /* R:  status for port 10 */
#define FTSWREG_STA9B    FTSWREG(0x4f)  /* R:  status for port 10 */

#define FTSWREG_DUMPO0   FTSWREG(0x58)  /* R:  encoded octet dump */
#define FTSWREG_DUMPO1   FTSWREG(0x59)  /* R:  encoded octet dump */
#define FTSWREG_DUMPO2   FTSWREG(0x5a)  /* R:  encoded octet dump */
#define FTSWREG_DUMPO3   FTSWREG(0x5b)  /* R:  encoded octet dump */
#define FTSWREG_DUMPO4   FTSWREG(0x5c)  /* R:  encoded octet dump */
#define FTSWREG_DUMPO5   FTSWREG(0x5d)  /* R:  encoded octet dump */
#define FTSWREG_DUMPO6   FTSWREG(0x5e)  /* R:  encoded octet dump */
#define FTSWREG_DUMPO7   FTSWREG(0x5f)  /* R:  encoded octet dump */
#define FTSWREG_DUMPK    FTSWREG(0x60)  /* R:  encoded K-symbol flag dump */
#define FTSWREG_REGDBG   FTSWREG(0x61)  /* RW: control debug info */
#define FTSWREG_TRGDBG   FTSWREG(0x62)  /* R:  debug info for trigger */
#define FTSWREG_DUMPI    FTSWREG(0x65)  /* R:  debug info for dump */
#define FTSWREG_DUMP0    FTSWREG(0x66)  /* R:  raw input dump */
#define FTSWREG_DUMP1    FTSWREG(0x67)  /* R:  raw input dump */
#define FTSWREG_DUMP2    FTSWREG(0x68)  /* R:  raw input dump */
#define FTSWREG_DUMP3    FTSWREG(0x69)  /* R:  raw input dump */
#define FTSWREG_DUMP4    FTSWREG(0x6a)  /* R:  raw input dump */
#define FTSWREG_DUMP5    FTSWREG(0x6b)  /* R:  raw input dump */
#define FTSWREG_DUMP6    FTSWREG(0x6c)  /* R:  raw input dump */
#define FTSWREG_DUMP7    FTSWREG(0x6d)  /* R:  raw input dump */
#define FTSWREG_DUMP8    FTSWREG(0x6e)  /* R:  raw input dump */
#define FTSWREG_DUMP9    FTSWREG(0x6f)  /* R:  raw input dump */

/* open mode */
#define FTSW_RDONLY    0x00
#define FTSW_RDWR      0x01
#define FTSW_USEFIFO   0x02

#define FTSW_VMEBASE  0x03000000
#define FTSW_VMEUNIT  0x00010000
#define FTSW_MMAPSIZE 0x00010000
#define SPARC_VMEDEV  "/dev/vme32d32"

#define M012_SERIAL      7
#define M012_SELECTMAP   6

#ifdef USE_LINUX_VME_UNIVERSE
#include <vme/vme.h>
#include <vme/vme_api.h>
#endif

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
ftsw_t* open_ftsw(int ftswid, int flag);
int close_ftsw(ftsw_t*);
int read_ftsw(ftsw_t*, int regid);
int readfifo_ftsw(ftsw_t*, unsigned* bufp);
int write_ftsw(ftsw_t*, int regid, int value);
void debug_ftsw(int value);
/* function for ftsw control node in NSM */
void write_ftsw_fpga(ftsw_t* ftsw, int m012, int ch, int n);
int boot_ftsw_fpga(ftsw_t* ftsw, const char* file,
                   int verbose, int forced, int m012);
void dump_ftsw_fpga(int conf, char* str);
int trigger_ftsw_single(ftsw_t* ftsw);
int stop_ftsw_trigger(ftsw_t* ftsw);
#ifdef __cplusplus
}
#endif

#endif
