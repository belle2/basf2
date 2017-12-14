/* ---------------------------------------------------------------------- *\

   ft2u041.h

   Definitions for the ft2u registers and a struct

   Mikihiko Nakao, KEK IPNS

   20130927 updated for ft2u023 / pocket_ttd
   20131207 missing CLKSEL was added

   ------------------------
   ft2u register definition
   ------------------------

   16 EXPRUN (RW)
      [31:22] (10-bit) experiment number
      [21:0]  (22-bit) run number

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
      [23:16] (8-bit) flip TDO polarity (to cope with faulty JTAG state)
      [14]    (1-bit)   TCK when bit-9 is '1'
      [13]    (1-bit)   TMS when bit-9 is '1'
      [12]    (1-bit)   TDI when bit-9 is '1'
      [9]     (1-bit)   1: JTAG from register [14:12], 0: from TESTPIN
      [8]     (1-bit) 1: auto JTAG port, send TCK/TMS/TDI to all port, but
                         use only TDO from the first port
          0: manual JTAG port
      [7:0]   (8-bit) enable each JTAG port, logical-or TDO ports is used

   1b JTDO (R)
      [0]   (1-bit) TDO status

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
      [31:28] (4-bit) tlumon: status of line to/from TLU
      [27]    (1-bit) tlubusy: busy is being sent to TLU
      [25]    (1-bit) tlutrg:  trigger signal from TLU
      [24]    (1-bit) tlurst:  reset signal from TLU
      [23:16] (8-bit) cnttlurst:  number of reset signals from TLU
      [14:0] (15-bit) tlutag: trigger tag from TLU

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

   30 ENSTAT (R)
      [27:24] octet counter
      [21:20] raw bit2
      [17]    is comma
      [16]    is K-symbol
      [15:8]  raw octet
      [6:4]   bit2 counter
      [3:0]   payload counter

   31 REVOPOS (R/W)
      (to be defined)

   32 REVOCAND (R)
      (revo-signal is yet to be defined)
      [31:16] revo-signal candidate counter
      [10:0]  revo-signal candidate position

   33 NOREVO (R)
      (revo-signal is yet to be defined)
      [31:16] off-timing revolution signal counter
      [15:0]  missing revolution signal counter

   34 BADDR (R/W)
      [20]    Broadcast bit for the reset signal
      [19:0]  Destination address for the reset signal

   37 LCKFREQ (R)
      [26:0]  on-board clock frequency

   38 DESTAT (R)
      [31] (fifoerr) FTSW's FIFO error
      [30] (pipebusy) FTSW's flow control pipeline busy
      [29] (sigbusy) logical-or of all external busy sources
      [28] (busy) logical-or of ALL busy sources
      [27:24]  (xnwff) COPPER's length FIFO full
      [23:20]  (xbusy) COPPER's slow busy
      [19:12]  (obusy) FEE's slow busy
      [11:8]   (xbusysig) COPPER's fast busy
      [7:0]    (obusysig) FEE's fast busy

      (fast busy based on single BUSY octet, slow busy based on busy
       bit in the packet)

   39 LINKUP (R)
      [23:20]  (xalive)  Connected COPPER is alive
      [19:12]  (oalive)  Connected FEE is alive
      [11:8]   (xlinkup) Connected COPPER's b2tt link is up
      [7:0]    (olinkup) Connected FEE's b2tt link is up

      (alive: meaningful b2tt packet is received
       linkup: linkup bit is on in the b2tt packet)

   3a ERROR (R)
      [31:30]  (staictrl) IDELAYCTRL status
      [11:8]   (xerr)  Error from COPPER
      [7:0]    (oerr)  Error from FEE

   3b LINKERR (R)
      [31:24]  (b2lup)  Belle2link is up at FEE
      [23:16]  (plllk)  MGT's PLL is locked at FEE
      [15]     (err)    any error
      [14]     (errin)  any error in from FEE
      [13]     (clkerr) error in clock phase detector
      [12]     (trigshort)  short interval trigger occured
      [11:8]   (xerr)   error in COPPER  (unused)
      [7:0]    (oerr)   error in FEE

   3c STA1A (R)
      status of OTRG port 1
      [31:12] addr
      [11]    feeerr
      [10]    tagerr or fifoerr
      [9]     seuerr
      [8]     busy
      [7:4]   tagdiff
      [3:0]   b2ldiff

   3d STA1B (R)
      [31:16] b2lcnt
      [15:0]  id

   (3e..4f repeats up to STA8A/STA8B)

   50 LATENCY
      [31:

   5a UDEAD (R)
      [31:0] (32-bit) total deadtime in second unit since counter or run reset.

   5b CDEAD (R)
      [26:0] (27-bit) total deadtime in clock unit since counter or run reset.

   5c PDEAD (R)
      [31:0] (32-bit) deadtime due to pipeline
      (this and following dead time has bit 31:16 count in second and
       bit 15:0 count in 2^11 clocks = 16 us)

   5d EDEAD (R)
      [31:0] (32-bit) deadtime due to error

   5e FDEAD (R)
      [31:0] (32-bit) deadtime due to FTSW's FIFO full

   5f RDEAD (R)
      [31:0] (32-bit) deadtime due to software register

   60--67 ODEAD (R)
      [31:0] (32-bit) deadtime due to FEE busy

   68--6b ODEAD (R)
      [31:0] (32-bit) deadtime due to COPPER busy

   80--87 DUMPO
      [31:0] (32-bit) octet dump (total 32 octets)

   88 DUMPK
      [31:0] (32-bit) K-symbol flag dump (total 32 octets)

   89 DUMPI
      [31:0] (32-bit) debug info for dump

   8a ACKSIG (R)
      [15:12]  (xack)  Raw ACK signal from COPPER
      [7:0]    (oack)  Raw ACK signal from FEE

   90-99 DUMP2
      [31:0] (32-bit) binary dump (total 32 octets)

   9e ERRSRC
      [3:0] (4-bit) error source (clkerr, errin, linkerr, tshort)
      [9:4] (6-bit) busy source
                    (busyin1, busyin2, trig, sig_busy1, regbusy, fullin)

   9f TDELAY
      [1:0]   (2-bit)  itrgsel - output to ITRG pair
      [5:4]   (2-bit)  irsvsel - output to IRSV pair
      [11:8]  (4-bit)  idump - dump source (0..7: OACK, 8..b: XACK, c: OTRG)
      [31:16] (16-bit) trgdelay

\* ---------------------------------------------------------------------- */

#include <stdint.h>

#define FT2U_FTSWID   FTSWREG_FTSWID  /* RW: "FTSW" identifier */
#define FT2U_CPLDVER  FTSWREG_CPLDVER /* R:  CPLD firmware version */
#define FT2U_CONF     FTSWREG_CONF    /* RW: for FPGA programming */
#define FT2U_CCLK     FTSWREG_CCLK    /* RW: for FPGA programming */
#define FT2U_CLKSEL   FTSWREG_CLKSEL  /* RW: clock select */
#define FT2U_SYSRST   FTSWREG_SYSRST  /* W:  sysreset */
#define FT2U_FPGAID   FTSWREG_FPGAID  /* RW: FPGA firmware type */
#define FT2U_FPGAVER  FTSWREG_FPGAVER /* R:  FPGA firmware version */

#define FT2U_SETUTIM  FTSWREG_SETUTIM /* RW: set utime */
#define FT2U_CLKFREQ  FTSWREG_CLKFREQ /* RW: # of clocks for 1 second */
#define FT2U_UTIME    FTSWREG_UTIME   /* R:  counted utime in sec */
#define FT2U_CTIME    FTSWREG_CTIME   /* R:  counted ctime in clock */
#define FT2U_RESET    FTSWREG_RESET   /* RW: run/count/b2link reset, etc */
#define FT2U_TRGSET   FTSWREG_TRGSET  /* RW: source, rate, opt */
#define FT2U_STAFIFO  FTSWREG_STAFIFO /* R:  FIFO status */
#define FT2U_FIFO     FTSWREG_FIFO    /* R:  FIFO of trigger data */

#define FT2U_EXPRUN   FTSWREG(0x16)  /* RW: 10-bit exp & 22-bit run */
#define FT2U_OMASK    FTSWREG(0x18)  /* RW: select RJ-45 port */
#define FT2U_STAMISC  FTSWREG(0x19)  /* R:  link status of RJ-45 port */
#define FT2U_JTAG     FTSWREG(0x1a)  /* RW: JTAG control */
#define FT2U_JTDO     FTSWREG(0x1b)  /* R:  JTAG TDO */
#define FT2U_JCTL     FTSWREG(0x1c)  /* RW: jitter cleaner control */
#define FT2U_JREG     FTSWREG(0x1d)  /* R:  jitter cleaner register */
#define FT2U_JRST     FTSWREG(0x1e)  /* RW: jitter cleaner reset */
#define FT2U_JSTA     FTSWREG(0x1f)  /* R:  jitter cleaner status */

#define FT2U_UTIMRST  FTSWREG(0x21)  /* R:  when reset happened */
#define FT2U_CTIMRST  FTSWREG(0x22)  /* R:  when reset happened */
#define FT2U_UTIMERR  FTSWREG(0x23)  /* R:  when error happened */
#define FT2U_CTIMERR  FTSWREG(0x24)  /* R:  when error happened */
#define FT2U_ERRSRC   FTSWREG(0x25)  /* R:  where error happened */
#define FT2U_TLUCTRL  FTSWREG(0x26)  /* RW: TLU control */
#define FT2U_TLUSTAT  FTSWREG(0x27)  /* R:  TLU status */

#define FT2U_TLIMIT   FTSWREG(0x29)  /* RW: # of trigger to generate */
#define FT2U_TINCNT   FTSWREG(0x2a)  /* R:  # of trigger-in since reset */
#define FT2U_TOUTCNT  FTSWREG(0x2b)  /* R:  # of trigger-out since reset */
#define FT2U_TLAST    FTSWREG(0x2c)  /* R:  # of trigger yet to generate */
/* FTSWREG(0x2f) is not used yet */

#define FT2U_ENSTAT   FTSWREG(0x30)  /* R:  encoder status */
#define FT2U_REVOPOS  FTSWREG(0x31)  /* RW: revosig position  */
#define FT2U_REVOIN   FTSWREG(0x32)  /* R:  revosig input status */
#define FT2U_REVOCNT  FTSWREG(0x33)  /* R:  bad revolution count */
#define FT2U_SETADDR  FTSWREG(0x34)  /* RW: set address for command */
#define FT2U_SETCMD   FTSWREG(0x35)  /* RW: command */
/* FTSWREG(0x36) is not used yet */
#define FT2U_LCKFREQ  FTSWREG(0x37)  /* R:  lck frequency */

#define FT2U_STAT     FTSWREG(0x38)  /* R:  output port status summary */
#define FT2U_LINKUP   FTSWREG(0x39)  /* R:  output port link status  */
#define FT2U_ERROR    FTSWREG(0x3a)  /* R:  output port error summary */
#define FT2U_LINKERR  FTSWREG(0x3b)  /* R:  link errors */
#define FT2U_STA0A    FTSWREG(0x3c)  /* R:  status for port 1 */
#define FT2U_STA0B    FTSWREG(0x3d)  /* R:  status for port 1 */
#define FT2U_STA1A    FTSWREG(0x3e)  /* R:  status for port 2 */
#define FT2U_STA1B    FTSWREG(0x3f)  /* R:  status for port 2 */
#define FT2U_STA2A    FTSWREG(0x40)  /* R:  status for port 3 */
#define FT2U_STA2B    FTSWREG(0x41)  /* R:  status for port 3 */
#define FT2U_STA3A    FTSWREG(0x42)  /* R:  status for port 4 */
#define FT2U_STA3B    FTSWREG(0x43)  /* R:  status for port 4 */
#define FT2U_STA4A    FTSWREG(0x44)  /* R:  status for port 5 */
#define FT2U_STA4B    FTSWREG(0x45)  /* R:  status for port 5 */
#define FT2U_STA5A    FTSWREG(0x46)  /* R:  status for port 6 */
#define FT2U_STA5B    FTSWREG(0x47)  /* R:  status for port 6 */
#define FT2U_STA6A    FTSWREG(0x48)  /* R:  status for port 7 */
#define FT2U_STA6B    FTSWREG(0x49)  /* R:  status for port 7 */
#define FT2U_STA7A    FTSWREG(0x4a)  /* R:  status for port 8 */
#define FT2U_STA7B    FTSWREG(0x4b)  /* R:  status for port 8 */
#define FT2U_STA8A    FTSWREG(0x4c)  /* R:  status for port 9 */
#define FT2U_STA8B    FTSWREG(0x4d)  /* R:  status for port 9 */
#define FT2U_STA9A    FTSWREG(0x4e)  /* R:  status for port 10 */
#define FT2U_STA9B    FTSWREG(0x4f)  /* R:  status for port 10 */

#define FT2U_XSTA1    FTSWREG(0x4c)  /* R:  status for TTRX port 1 */
#define FT2U_XSTA2    FTSWREG(0x4d)  /* R:  status for TTRX port 2 */
#define FT2U_XSTA3    FTSWREG(0x4e)  /* R:  status for TTRX port 3 */
#define FT2U_XSTA4    FTSWREG(0x4f)  /* R:  status for TTRX port 4 */

#define FT2U_MAXT0    FTSWREG(0x50)  /* RW: max trig / latency for port 0 */
#define FT2U_MAXT1    FTSWREG(0x51)  /* RW: max trig / latency for port 1 */
#define FT2U_MAXT2    FTSWREG(0x52)  /* RW: max trig / latency for port 2 */
#define FT2U_MAXT3    FTSWREG(0x53)  /* RW: max trig / latency for port 3 */
#define FT2U_MAXT4    FTSWREG(0x54)  /* RW: max trig / latency for port 4 */
#define FT2U_MAXT5    FTSWREG(0x55)  /* RW: max trig / latency for port 5 */
#define FT2U_MAXT6    FTSWREG(0x56)  /* RW: max trig / latency for port 6 */
#define FT2U_MAXT7    FTSWREG(0x57)  /* RW: max trig / latency for port 7 */
#define FT2U_MAXT8    FTSWREG(0x58)  /* RW: max trig / latency for port 8 */
#define FT2U_MAXT9    FTSWREG(0x59)  /* RW: max trig / latency for port 9 */

/* ft2u only */
#define FT2U_LATENCY  FTSWREG(0x50)  /* RW: max trig / latency */
#define FT2U_TLU      FTSWREG(0x52)  /* R: various TLU status */
#define FT2U_UTIMTLU  FTSWREG(0x53)  /* R: TLU reset utime */
#define FT2U_CTIMTLU  FTSWREG(0x54)  /* R: TLU reset ctime */

#define FT2U_UDEAD    FTSWREG(0x5a)  /* R:  deadtime in sec */
#define FT2U_CDEAD    FTSWREG(0x5b)  /* R:  deadtime in clock */

/* following dead time has (utime[15:0] << 16 | ctime[26:11]) */
#define FT2U_PDEAD    FTSWREG(0x5c)  /* R:  pipeline deadtime */
#define FT2U_EDEAD    FTSWREG(0x5d)  /* R:  error deadtime */
#define FT2U_FDEAD    FTSWREG(0x5e)  /* R:  FTSW fifoful deadtime */
#define FT2U_RDEAD    FTSWREG(0x5f)  /* R:  software (sendtoeb) deadtime */
#define FT2U_ODEAD0   FTSWREG(0x60)  /* R:  port 0 deadtime */
#define FT2U_ODEAD1   FTSWREG(0x61)  /* R:  port 1 deadtime */
#define FT2U_ODEAD2   FTSWREG(0x62)  /* R:  port 2 deadtime */
#define FT2U_ODEAD3   FTSWREG(0x63)  /* R:  port 3 deadtime */
#define FT2U_ODEAD4   FTSWREG(0x64)  /* R:  port 4 deadtime */
#define FT2U_ODEAD5   FTSWREG(0x65)  /* R:  port 5 deadtime */
#define FT2U_ODEAD6   FTSWREG(0x66)  /* R:  port 6 deadtime */
#define FT2U_ODEAD7   FTSWREG(0x67)  /* R:  port 7 deadtime */
#define FT2U_XDEAD1   FTSWREG(0x68)  /* R:  copper 1 deadtime */
#define FT2U_XDEAD2   FTSWREG(0x69)  /* R:  copper 2 deadtime */
#define FT2U_XDEAD3   FTSWREG(0x6a)  /* R:  copper 3 deadtime */
#define FT2U_XDEAD4   FTSWREG(0x6b)  /* R:  copper 4 deadtime */

#define FT2U_ODBG0    FTSWREG(0x70)  /* R:  port 0 deadtime */
#define FT2U_ODBG1    FTSWREG(0x71)  /* R:  port 1 deadtime */
#define FT2U_ODBG2    FTSWREG(0x72)  /* R:  port 2 deadtime */
#define FT2U_ODBG3    FTSWREG(0x73)  /* R:  port 3 deadtime */
#define FT2U_ODBG4    FTSWREG(0x74)  /* R:  port 4 deadtime */
#define FT2U_ODBG5    FTSWREG(0x75)  /* R:  port 5 deadtime */
#define FT2U_ODBG6    FTSWREG(0x76)  /* R:  port 6 deadtime */
#define FT2U_ODBG7    FTSWREG(0x77)  /* R:  port 7 deadtime */
#define FT2U_XDBG1    FTSWREG(0x78)  /* R:  copper 1 deadtime */
#define FT2U_XDBG2    FTSWREG(0x79)  /* R:  copper 2 deadtime */
#define FT2U_XDBG3    FTSWREG(0x7a)  /* R:  copper 3 deadtime */
#define FT2U_XDBG4    FTSWREG(0x7b)  /* R:  copper 4 deadtime */


#define FT2U_DUMPO0   FTSWREG(0x80)  /* R:  encoded octet dump */
#define FT2U_DUMPO1   FTSWREG(0x81)  /* R:  encoded octet dump */
#define FT2U_DUMPO2   FTSWREG(0x82)  /* R:  encoded octet dump */
#define FT2U_DUMPO3   FTSWREG(0x83)  /* R:  encoded octet dump */
#define FT2U_DUMPO4   FTSWREG(0x84)  /* R:  encoded octet dump */
#define FT2U_DUMPO5   FTSWREG(0x85)  /* R:  encoded octet dump */
#define FT2U_DUMPO6   FTSWREG(0x86)  /* R:  encoded octet dump */
#define FT2U_DUMPO7   FTSWREG(0x87)  /* R:  encoded octet dump */
#define FT2U_DUMPK    FTSWREG(0x88)  /* R:  encoded K-symbol flag dump */
#define FT2U_DUMPI    FTSWREG(0x89)  /* R:  debug info for dump */
#define FT2U_ACKSIG   FTSWREG(0x8a)  /* R:  raw ACK signal */

#define FT2U_DUMP0    FTSWREG(0x90)  /* R:  raw input dump */
#define FT2U_DUMP1    FTSWREG(0x91)  /* R:  raw input dump */
#define FT2U_DUMP2    FTSWREG(0x92)  /* R:  raw input dump */
#define FT2U_DUMP3    FTSWREG(0x93)  /* R:  raw input dump */
#define FT2U_DUMP4    FTSWREG(0x94)  /* R:  raw input dump */
#define FT2U_DUMP5    FTSWREG(0x95)  /* R:  raw input dump */
#define FT2U_DUMP6    FTSWREG(0x96)  /* R:  raw input dump */
#define FT2U_DUMP7    FTSWREG(0x97)  /* R:  raw input dump */
#define FT2U_DUMP8    FTSWREG(0x98)  /* R:  raw input dump */
#define FT2U_DUMP9    FTSWREG(0x99)  /* R:  raw input dump */

#define FT2U_DUMPC    FTSWREG(0x9c)  /* RW: dump control */
#define FT2U_CRC8     FTSWREG(0x9d)  /* R:  crc8 monitor */
#define FT2U_ERRSRC2  FTSWREG(0x9e)  /* R:  error source */
#define FT2U_TDELAY   FTSWREG(0x9f)  /* RW: trigger delay / itrg out sel */

static const int ft2u_revision = 6;

struct ft2u {
  uint8_t isrunning;
  uint8_t sigpipe;
  uint8_t verbose;
  uint8_t ebisbusy;
  /* tmp */
  int16_t b2ebport;
  int16_t b2ebsock;
  int32_t b2ebip;
  int32_t myip;
  int32_t exp_number;
  int32_t run_number;
  int32_t run_count;
  /* int32_t evt_number; */
  /* int32_t evt_total; */
  /* FTSW registers */
  int32_t ftswid;   /* FT2U_FTSWID  00(RW) "FTSW" id */
  int32_t cpldver;  /* FT2U_CPLDVER 01(R)  CPLD version */
  int32_t conf;     /* FT2U_CONF    02(RW) FPGA program */
  int32_t cclk;     /* FT2U_CCLK    03(RW) FPGA CCLK */
  int32_t clksel;   /* FT2U_CLKSEL  04(RW) clock select */
  int32_t fpgaid;   /* FT2U_FPGAID  10(RW) FPGA type */
  int32_t fpgaver;  /* FT2U_FPGAVER 11(R)  FPGA version */
  int32_t setutim;  /* FT2U_SETUTIM 12(RW) set utime */
  int32_t clkfreq;  /* FT2U_CLKFREQ 13(RW) # clocks/1s */
  int32_t utime;    /* FT2U_UTIME   14(R)  utime in sec */
  int32_t ctime;    /* FT2U_CTIME   15(R)  ctime in clock */
  int32_t exprun;   /* FT2U_EXPRUN  16(RW) 10-bit exp & 22-bit run */
  int32_t omask;    /* FT2U_OMASK   18(RW) RJ-45 port */
  int32_t stamisc;  /* FT2U_STAMISC 19(R)  link status */
  int32_t jtag;     /* FT2U_JTAG    1a(RW) JTAG control */
  int32_t jtdo;     /* FT2U_JTDO    1b(R)  JTAG TDO */
  int32_t jctl;     /* FT2U_JCTL    1c(RW) jitter control */
  int32_t jreg;     /* FT2U_JREG    1d(R)  jitter register */
  int32_t jrst;     /* FT2U_JRST    1e(RW) jitter reset */
  int32_t jsta;     /* FT2U_JSTA    1f(R)  jitter status */
  int32_t reset;    /* FT2U_RESET   20(RW) reset */
  int32_t utimrst;  /* FT2U_UTIMRST 21(R)  reset utime */
  int32_t ctimrst;  /* FT2U_CTIMRST 22(R)  reset ctime */
  int32_t utimerr;  /* FT2U_UTIMERR 23(R)  error utime */
  int32_t ctimerr;  /* FT2U_CTIMERR 24(R)  error ctime */
  int32_t errsrc;   /* FT2U_ERRSRC  25(R)  error source */
  int32_t trgset;   /* FT2U_TRGSET  28(RW) gentrig set */
  int32_t tlimit;   /* FT2U_TLIMIT  29(RW) # gentrig */
  int32_t tincnt;   /* FT2U_TINCNT  2a(R)  # trigger-in */
  int32_t toutcnt;  /* FT2U_TOUTCNT 2b(R)  # trigger-out */
  int32_t tlast;    /* FT2U_TLAST   2c(R)  # trigger last */
  int32_t stafifo;  /* FT2U_STAFIFO 2d(R)  FIFO status */
  int32_t enstat;   /* FT2U_ENSTAT  30(R)  encoder status */
  int32_t revopos;  /* FT2U_REVOPOS 31(RW) revosig pos */
  int32_t revoin;   /* FT2U_REVOIN  32(R)  revosig status */
  int32_t revocnt;  /* FT2U_REVOCNT 33(R)  bad revo */
  int32_t setaddr;  /* FT2U_SETADDR 34(RW) set address */
  int32_t setcmd;   /* FT2U_SETCMD  35(RW) command */
  int32_t lckfreq;  /* FT2U_LCKFREQ 37(R)  lck frequency */
  int32_t stat;     /* FT2U_STAT    38(R)  port status */
  int32_t linkup;   /* FT2U_LINKUP  39(R)  link status */
  int32_t error;    /* FT2U_ERROR   3a(R)  port error */
  int32_t linkerr;  /* FT2U_LINKERR 3b(R)  link error */
  int32_t staa[8];  /* FT2U_STA[0..7]A  3c..4a(R)  status port 1-8 */
  int32_t stab[8];  /* FT2U_STA[0..7]B  3d..4b(R)  status port 1-8 */
  int32_t xsta[4];  /* FT2U_XSTA[1..4]  4c..4f(R)  status ttrx 1-4 */
  /* int32_t staa[10]; /* FT2U_STA[0..9]A  3c..4e(R)  status port 1-10 */
  /* int32_t stab[10]; /* FT2U_STA[0..9]B  3d..4f(R)  status port 1-10 */
  /* int32_t xsta[4];  /* FT2U_XSTA[1..4]  4c..4f(R)  status ttrx 1-4 */
  /* int32_t maxt[10]; /* FT2U_MAXT[0..9]  50..59(RW) flow control 1-10 */
  int32_t latency;  /* FT2U_LATENCY 50 flow control (ft2u) */
  int32_t tlu;      /* FT2U_TLU     52 TLU (ft2u) */
  int32_t utimtlu;  /* FT2U_UTIMTLU 53 TLU reset utime (ft2u) */
  int32_t ctimtlu;  /* FT2U_CTIMTLU 54 TLU reset ctime (ft2u) */
  int32_t udead;    /* FT2U_UDEAD   5a(R)  deadtime sec */
  int32_t cdead;    /* FT2U_CDEAD   5b(R)  deadtime clock */
  /* following dead time has (utime[15:0] << 16 | ctime[26:11]) */
  int32_t pdead;    /* FT2U_PDEAD   5c(R)  pipeline deadtime */
  int32_t edead;    /* FT2U_EDEAD   5d(R)  error deadtime */
  int32_t fdead;    /* FT2U_FDEAD   5e(R)  FTSW fifoful deadtime */
  int32_t rdead;    /* FT2U_RDEAD   5f(R)  software (sendtoeb) deadtime */
  int32_t odead[8]; /* FT2U_ODEAD   60-67(R)  port 1-8 deadtime */
  int32_t xdead[4]; /* FT2U_XDEAD   68-6b(R)  port 1-8 deadtime */
  int32_t odbg[8];  /* FT2U_ODBG    70-77(R)  port 1-8 deadtime */
  int32_t xdbg[4];  /* FT2U_XDBG    78-7b(R)  port 1-8 deadtime */
  int32_t dumpo[8]; /* FT2U_DUMPO   80-87(R)  dump octet */
  int32_t dumpk;    /* FT2U_DUMPK   88(R)     dump K-symbol flag */
  int32_t dumpi;    /* FT2U_DUMPI   89(R)     dump info */
  int32_t acksig;   /* FT2U_ACKSIG  8a(R)  raw ACK signal */
  int32_t dump[10]; /* FT2U_DUMP    90-99(R)  raw stream dump */
  int32_t tdelay;   /* FT2U_TDELAY  9f(RW)  delay */
};

typedef struct ft2u ft2u_t;
