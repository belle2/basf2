/* ---------------------------------------------------------------------- *\

   ttrx.h

   Definitions for the TT-RX registers and a few utility functions

   Mikihiko Nakao, KEK IPNS
   Eunil Won, Korea Univ

   $Id: ttrxlib.h,v 1.1.1.1 2005/02/23 09:44:12 ttrx Exp $
   $Log: ttrxlib.h,v $
   Revision 1.1.1.1  2005/02/23 09:44:12  ttrx
   First into cvs


   version
   2003110400  working version
   2004083100  TT-RX v4
   2004090700  TT-RX v4 (FPGA v0.01)


   -------------------------
   TT Rx register definition
   -------------------------

   1. Generic idea

   - 12-bit address, 32-bit data space is used (A12D32)

   - Some address bits are not decoded, but do not assume this feature.
     .. A[15:12] address lines are not decoded.
     .. A[3:2] address lines are not decoded for most of the registers
        (exception: ttrx_fifo1 and ttrx_fifo2)
     .. For ttrx_fifo1 and ttrx_fifo2, A[7:3] are not decoded.

   - Address space 0x000 - 0x07f is used by CPLD
     Address space 0x080 - 0xfff is used by FPGA

   - Only 32-bit data access is supported
     .. A[1:0] must be always 0
     .. FPGA provides full 32-bit registers
        CPLD provides only 4-bit registers (bit 3 to 0)

   2. Registers
   ----------------------------------------------------------------------
   0x000 - TTRX_PID (CPLD id)                                         (R)
   - always 0x3
   ----------------------------------------------------------------------
   0x010 - TTRX_PVER (CPLD version)                                   (R)
   - minor version number, start from 1 (0 is not allowed)
   ----------------------------------------------------------------------
   0x020 - TTRX_PRGM (FPGA program register)                        (R/W)
   - Read
    [bit 0] done == 1 => FPGA is programmed
    [bit 1] init == 1 => FPGA programming has started
    [bit 2] m012 (set to 0 by poweron/reset)
    [bit 3] t_lock_b == 0 => serial-bus is locked (okay)
    (0x3 or 0x7 is okay, 0xb or 0xf if serial-bus is disconnected)
   - Write
    0x2    f_cclk/din=0 - generate f_cclk with din=0
    0x3    f_cclk/din=1 - generate f_cclk with din=1
    0x4    m012=0 - FPGA programming from EPROM
    0x5    m012=1 - FPGA programming from CPLD
    0x6    prgm=1 - start FPGA programming (cleared when done=0)
    0xe    reset (entire TT-RX reset)

   ----------------------------------------------------------------------
   0x030 - TTRX_SCLK (serial-bus clock control)                     (R/W)
    [bit 0] edgeo (default = 0)
    [bit 1] edgei (default = 0)
    [bit 2] tclken (readonly, 0 = local clock, 1 = serial clock)
    [bit 3] tclksel (default = 1 = serial clock if available)

   ----------------------------------------------------------------------
   0x080 - TTRX_RCLK (reduced clock and trigin control)             (R/W)
    [bit 7:0] rdiv: reduced clock setting, rclk = sclk/(rdiv + 1)
    [bit 8]   ttrig (readonly)
    [bit 9]   localtrg (readonly)
    [bit 10]  trig (readonly)
    [bit 11]  trigin (readonly)
    [bit 15:12] ext (readonly)

   ----------------------------------------------------------------------
   0x100 - TTRX_ID (TTRX/FPGA id)                                   (R/W)
   - 0x52585454 ("TTRX" in little endian)
   - Any value can be written on this register to test the read, write
     and reset features of the FPGA part.

   ----------------------------------------------------------------------
   0x110 - TTRX_VER (TTRX/FPGA version)                               (R)
   [bit 31:28] TTRX version = 4
   [bit 27:16] FPGA major version = 0 for now
   [bit 15:0]  FPGA minor version

   ----------------------------------------------------------------------
   0x120 - TTRX_CREG (COPPER control/status register)               (R/W)

   [bit  0]  read:  FIFO empty
             write: FIFO reset
   [bit  1]  read:  FIFO halffull
   [bit  2]  read:  FIFO full
   [bit  3]  read:  FIFO overrun  (should not happen)
   [bit  4]  read:  level of software busy (should be 0)
             write: generate software busy pulse
   [bit  5]  read:  level of test busy (for debug)
   [bit  6]  read:  nwff - number of word fifo full
   [bit  7]  read:  ready
   [bit  11:8]  read: busy handshake done (not for use)
   [bit 15:12]  read: extra busy
   [bit 15]  write: write 1 to clear extra busy
   [bit 16]  r/w:   enable trigger-stop at COPPER FIFO full (FINESSE & NWFF)
   [bit 17]  r/w:   enable test busy

   [bit 18]  r/w:   COPPER abrt
   [bit 19]  r/w:   COPPER iena
   [bit 20]  r/w:   COPPER irst
   [bit 21]  r/w:   COPPER srst
   [bit 22]  r/w:   COPPER frst
   [bit 23]  read:  COPPER of revo signal

   [bit 24]  r/w:   disable trigger-stop at TT-RX FIFO half
   [bit 25]  r/w:   disable trigger-stop at TT-RX FIFO full

   [bit 26]  r/w:   enable COPPER abrt
   [bit 27]  r/w:   enable COPPER iena
   [bit 28]  r/w:   enable COPPER irst
   [bit 29]  r/w:   enable COPPER srst
   [bit 30]  r/w:   enable COPPER frst
   [bit 31]  r/w:   enable COPPER revo

   ----------------------------------------------------------------------
   0x0130 - TTRX_FREG (FINESSE control/status register)               (R)
   - Read
    |3322|2222|2222|1111|  |1111|1100|0000|0000|
    |1098|7654|3210|9876|  |5432|1098|7654|3210|
    +----+----+----+----+  +----+----+----+----+
    |io2 |io1 |ful |emp |  |bsy |hand|flg |mask|
    |dcba|dcba|dcba|dcba|  |dcba|dcba|dcba|dcba|

    Bit mod 4 = 0 for FINESS A
    Bit mod 4 = 1 for FINESS B
    Bit mod 4 = 2 for FINESS C
    Bit mod 4 = 3 for FINESS D

    io2 [bit 31:28] for reserved line 2
    io1 [bit 27:24] for reserved line 1
    ful [bit 23:20] for FIFO full
    emp [bit 19:16] for FIFO empty
    bsy [bit 15:12] for BUSY  (bsy & hand => "00" => "01" => "11" => "00")
                                           (i.e. "10" from an orphan BUSY)
    hand [bit 11:8] for In handshake (i.e. trigger should be received)
    flg   [bit 7:4] ignore busy bit
    mask  [bit 3:0] for disable

   - Write
    |3322|2222|2222|1111|  |1111|1100|0000|0000|
    |1098|7654|3210|9876|  |5432|1098|7654|3210|
    +----+----+----+----+  +----+----+----+----+
    |io2 |io1 |    |    |  |    |    |    |mask|
    |dcba|dcba|    |    |  |    |    |    |dcba|

   ----------------------------------------------------------------------
   0x0140 - TTRX_INTR (interrupt request)                           (R/W)
   - [bit 0] not l_int_b
     (PCI interrupt is generated when bit 0 is asserted to 1)

   ----------------------------------------------------------------------
   0x0200 - TTRX_TRIG (trigger generator)                           (R/W)
   - Poweron/reset: 0x00000000
   - [bit 23..0] (ntrig)
     == 0x0000000 => generate triggers for ever
     >  0x0000000 => generate triggers for given count
   - [bit 26] == 1 => generate free mode (read only)
   - [bit 27] == 1 => generate random trigger
   - [bit 28] == 1 => generate pulse trigger
   - [bit 29] == 1 => enable user trigger;    0 => disable
                     (user trigger may put in from EXT connector)
   - [bit 30] == 0 => decrement ntrig for correct handshake (check busy)
              == 1 => decrement ntrig for generated trigger (no busy check)
   - [bit 31] == 1 => disable trigger; 0 => enable
                     (when ntrig triggers are generated, this bit turns on)

   ----------------------------------------------------------------------
   0x0210 - TTRX_TLAST (remaining number of local trigger)            (R)
   - Poweron/reset: 0x00000000
   - [bit 23..0] (nlast)

   ----------------------------------------------------------------------
   0x0220 - TTRX_RATE (trigger rate setting)                        (R/W)
   - Poweron/reset: 0x00000000
   - [bit 9:0]   plsset:  1/(N+1) prescale of ref clock, up to 1/1024
   - [bit 17:16] rndset:  (forgotten the meaning...)
   - [bit 28:24] basesel: reference clock selector, 0:42.33MHz to 20:40Hz

   ----------------------------------------------------------------------
   0x0230 - TTRX_TAG (trigger tag status)                             (R)
   - trigger tag for the last received trigger
     (lowest 8 bit are the used for the s_tag signal)
   ----------------------------------------------------------------------
   0x0250 - TTRX_DEPTH (FIFO depth)                                   (R)
   - number of events in the FIFO (upto 255)

   ----------------------------------------------------------------------
   0x0300 - TTRX_FIFO1                                                (R)
   - 32bit trigger tag if both FIFOs are not empty
   - 0xffffffff if either FIFO is empty

   ----------------------------------------------------------------------
   0x0314 - TTRX_FIFO2                                                (R)
   - should be  0 if FIFO is not empty
   - should be -1 if FIFO is empty

   (in the final version, this will contain more information:
    - [bit 13:0]  => bunch number (up to 3*5120-1)
    - [bit 19:16] => trigger type)

   ----------------------------------------------------------------------
   0x0400 - TTRX_TIO (test purpose)                                 (R/W)
   - [bit 9:0]   tout_usr
   - [bit 19:10] tout_raw
   - [bit 29:20] tin_raw
   - [bit 31]    entout_usr (=1 to use tout_usr out)

\* ---------------------------------------------------------------------- */

/* register map */
#define TTRX_PID     (0x000 >> 2)
#define TTRX_PVER    (0x010 >> 2)
#define TTRX_PRGM    (0x020 >> 2)
#define TTRX_SCLK    (0x030 >> 2)
#define TTRX_RCLK    (0x080 >> 2)

#define TTRX_ID      (0x100 >> 2)
#define TTRX_VER     (0x110 >> 2)
#define TTRX_CREG    (0x120 >> 2)
#define TTRX_FREG    (0x130 >> 2)
#define TTRX_INTR    (0x140 >> 2)

#define TTRX_TRIG    (0x200 >> 2)
#define TTRX_TLAST   (0x210 >> 2)
#define TTRX_RATE    (0x220 >> 2)
#define TTRX_TAG     (0x230 >> 2)
#define TTRX_TAGR    (0x240 >> 2)
#define TTRX_DEPTH   (0x250 >> 2)
#define TTRX_FIFO1   (0x300 >> 2)
#define TTRX_FIFO2   (0x314 >> 2) /* to be compatible with both v3 and v4 */

#define TTRX_TIO     (0x400 >> 2)
#define TTRX_TTEST   (0x410 >> 2)
#define TTRX_TSTAT   (0x420 >> 2)
#define TTRX_TSAV    (0x430 >> 2)
#define TTRX_TSAV2   (0x440 >> 2) /* temporary */
#define TTRX_TNG32   (0x450 >> 2) /* temporary */

/* library functions */
extern int get_ttrx(unsigned int* base_plx_p, unsigned int* base_ttrx_p,
                    int ttrxid);
extern unsigned int* maprw_ttrx(unsigned base, unsigned size);
extern unsigned int* mapro_ttrx(unsigned base, unsigned size);
extern void reset_ttrx(volatile unsigned int* ttrx_p);
