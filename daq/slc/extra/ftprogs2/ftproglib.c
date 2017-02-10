/* ---------------------------------------------------------------------- *\

   ftproglib.c - commonly used ftprogs functions

   Mikihiko Nakao, KEK IPNS

   2016.0413.0955 new
   
\* ---------------------------------------------------------------------- */

static const char *V = "ftproglib 2016.0413.0955";

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

#include "ftsw.h"
#include "ftreg_reset.h"
#include "ftproglib.h"

/* -- utimeft -------------------------------------------------------- *\
\* ------------------------------------------------------------------- */
void
utimeft(ftsw_t *ftsw, const char *PROGRAM)
{
  /* fpga id/ver dependent */
  uint32_t fpgaid  = read_ftsw(ftsw, FTSWREG_FPGAID);
  uint32_t fpgaver = read_ftsw(ftsw, FTSWREG_FPGAVER) & 0x3ff;
  
  /* ft2u and ft3u version 0.17 and later: trgset, utime, reset setup */
  if ((fpgaid == 0x46543255 && fpgaver >= 17) ||  /* FT2U */
      (fpgaid == 0x46543355 && fpgaver >= 17) ||  /* FT3U */
      (fpgaid == 0x4654324f && fpgaver >=  2) ||  /* FT2O */
      (fpgaid == 0x4654334f && fpgaver >=  2) ||  /* FT3O */
      (fpgaid == 0x46543250 && fpgaver >=  2) ||  /* FT2P */
      (fpgaid == 0x46543350 && fpgaver >=  2)) {  /* FT3P */
    struct timeval tv;
    int n;
    int sec;
    
    write_ftsw(ftsw, FTSWREG_TRGSET, 0);
      
    gettimeofday(&tv, 0);
    sec = tv.tv_sec;
      
    for (n=0; ; n++) {
      gettimeofday(&tv, 0);
      if (sec != tv.tv_sec) {
        write_ftsw(ftsw, FTSWREG_SETUTIM, tv.tv_sec);
        if (PROGRAM) {
          printf("%s: utime is set, n=%d sec=%ld usec=%ld\n",
                 PROGRAM, n, tv.tv_sec, tv.tv_usec);
        }
        break;
      }
    }
    /* nofifo, trgstop, runreset */
    write_ftsw(ftsw, FTSWREG_RESET, 0x80000009); 
        
  } /* if fpgaid */
}
/* -- resetft -------------------------------------------------------- *\
   resetft(ftsw,1,0,0,0,0,0,"resetft") for "resetft" command
\* ------------------------------------------------------------------- */
uint32_t
resetft(ftsw_t *ftsw,
        int doreset,
        int dofifo,
        int dowait,
        int dobor,
        int notagerr,
        int trgtype,
        const char *PROGRAM)
{
  int reg_reset = read_ftsw(ftsw, FTSWREG_RESET);
  int iserror   = 0;
  int isclkerr  = 0;
  int fpgaid  = read_ftsw(ftsw, FTSWREG_FPGAID) & ~0x100; /* 3 => 2 */
  int fpgaver = read_ftsw(ftsw, FTSWREG_FPGAVER) & 0x3ff;
  int trigver = (fpgaid != 0x46543255) ? 2 : (fpgaver >= 40 ? 1 : 0);

  if (trigver == 1) {
    int reg_err  = read_ftsw(ftsw, FTSWREG(0x3a));
    if ((reg_err  >> 13) & 1) isclkerr = 1;
  } else if (trigver == 2) {
    int reg_err2 = read_ftsw(ftsw, FTSWREG(0x39));
    if ((reg_err2 >> 30) & 1) isclkerr = 1;
  }
  if (isclkerr) utimeft(ftsw, PROGRAM);

  if (1)            reg_reset |=  FTSWBIT_BUSY;     // bit16
  if (doreset)      reg_reset |=  FTSWBIT_RUNRESET; // bit0
  if (! dofifo)     reg_reset |=  FTSWBIT_NOFIFO;   // bit31
  if (dofifo)       reg_reset &= ~FTSWBIT_NOFIFO;   // bit31
  if (dowait)       reg_reset |=  FTSWBIT_RUNNING;  // bit20
  if (! dowait)     reg_reset &= ~FTSWBIT_RUNNING;  // bit20
  if (dobor && trgtype != 3) {
    reg_reset |= FTSWBIT_GENBOR; // bit2
  } else {
    reg_reset &= ~FTSWBIT_GENBOR; // bit2
  }
  if (notagerr)  reg_reset |= FTSWBIT_NOTAGERR; // bit29
  if (trgtype == 3) {
    /* fixed delay for a 15m cable
       (for a shorter cable, set TLUDELAY to 0 or 1) */
    reg_reset |=  (FTSWBIT_USETLU | FTSWBIT_TLUDELAY(2)); /* TLU */
  } else {
    reg_reset &= ~(FTSWBIT_USETLU | FTSWBIT_TLUDELAY(2)); /* TLU */
  }
  write_ftsw(ftsw, FTSWREG_RESET, reg_reset | FTSWBIT_TRGSTOP);
  
  usleep(100); /* 100usec = one revo cycle */

  write_ftsw(ftsw, FTSWREG_TLIMIT, 0);

  reg_reset &= ~FTSWBIT_RUNRESET;  /* no runreset */
  reg_reset &= ~FTSWBIT_BUSY;      /* no busy */
  write_ftsw(ftsw, FTSWREG_RESET, reg_reset | FTSWBIT_ERRRESET);

  if (trigver == 1) {
    int reg_err  = read_ftsw(ftsw, FTSWREG(0x3a));
    if ((reg_err  >> 15) & 1) iserror  = 1;
    if ((reg_err  >> 13) & 1) isclkerr = 1;
  } else if (trigver == 2) {
    int reg_err  = read_ftsw(ftsw, FTSWREG(0x3b));
    int reg_err2 = read_ftsw(ftsw, FTSWREG(0x39));
    if ((reg_err  >> 31) & 1) iserror  = 1;
    if ((reg_err2 >> 30) & 1) isclkerr = 1;
  }

  if (iserror && PROGRAM) {
    if (isclkerr) {
      printf("%s: clock error, cannot start, better to bootft again\n",
             PROGRAM);
    } else {
      printf("%s: FTSW could not reset error\n", PROGRAM);
    }
  }
  if (iserror) {
    return (uint32_t)-1;
  } else {
    return reg_reset;
  }
}
