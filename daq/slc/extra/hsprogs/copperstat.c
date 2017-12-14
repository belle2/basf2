/* ------------------------------------------------------------------ *\
 *
 *  copperstat.c
 *
 *  COPPER status flags from ioctl
 *
 *  Mikihiko Nakao (KEK)
 *  
 *  0.01  20130906  first version
 *  0.02  20140705  magic2
 *
\* ------------------------------------------------------------------ */

#define VERSION 0.02

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>

#include "copper.h"
#include <string.h>
#include <time.h>   // for getdate and gettime
#include <unistd.h> // for getdate and gettime

#ifdef STDINT_H_IS_MISSING
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

const char *PROGRAM = "copperstat";
int SIGNALED = 0;

/* ------------------------------------------------------------------ *\
   getcopper
\* ------------------------------------------------------------------ */
int
getcopper(int cprfd, int request, const char *name)
{
  int ret;
  int val;

  if ((ret = ioctl(cprfd, request, &val)) < 0) {
    printf("can't ioctl: %s\n", strerror(errno));
    exit(1);
  }
  printf("%03x %-12.12s %08x", request & 0xff, name, val);

  switch (request) {
  case CPRIOGET_FF_STA:
    printf(" 23:empty 22:almost-e 24:some-data 2c:almost-f 3c:full");
    break;
  case CPRIOGET_CONF_WA_AE:
    printf(" almost empty threshold A (%d words, %d%%)",
	   val, val*100/256/1024);
    break;
  case CPRIOGET_CONF_WA_FF:
    printf(" some data threshold A (%d words, %d%%)",
	   val, val*100/256/1024);
    break;
  case CPRIOGET_CONF_WA_AF:
    printf(" almost full threshold A (%d words, %d%%)",
	   val, val*100/256/1024);
    break;
  case CPRIOGET_FF_RST:
    printf(" (W only) reset event_fifo(bit 3:0)/finesse(bit 4)");
    break;
  case CPRIOGET_FINESSE_STA:
    printf(" FINESSE enable (bit 3:0)");
    break;
  case CPRIOGET_ALMFULL_ENB:
    printf(" length-FIFO full (NWFF) disable (bit 0)");
    break;
  case CPRIOGET_VERSION:
    printf(" firmware version");
    break;
  case CPRIOGET_TRGCM:
    printf(" on-board trigger (bit-0: single, bit-1: continuous)");
    break;
  case CPRIOGET_TRG_LENG:
    printf(" on-board trigger interval");
    break;
  case CPRIOGET_LEF_READSEL:
    printf(" bit-0: multiple length-FIFO read");
    break;
  case CPRIOGET_LEF_TOTAL:
    printf(" length-FIFO total");
    break;
  case CPRIOGET_LEF_AB:
    printf(" length-FIFO B(bit 31:16) / A(bit 15:0)");
    break;
  case CPRIOGET_LEF_CD:
    printf(" length-FIFO D(bit 31:16) / C(bit 15:0)");
    break;
  case CPRIOGET_LEF_STA:
    printf(" 4-bit per FINESSE 1:empty 2:some-data 6:almost e:full");
    break;
  case CPRIOGET_LEF_WA_FF:
    printf(" length-FIFO some data threshold A");
    break;
  case CPRIOGET_LEF_WA_AF:
    printf(" length-FIFO almost full threshold A");
    break;
  case CPRIOGET_INT_STA:
    printf(" intr bit 0:any-AF, 1:any-LEF-AF, 2:all-LEF, 3:fifo");
    break;
  case CPRIOGET_INT_MASK:
    printf(" intr mask");
    break;
  case CPRIOGET_EWRP_WA:
    printf(" data-FIFO read/write pointer");
    break;
  case CPRIOGET_WEA_COUNTER:
    printf(" data-FIFO write counter = # of written words");
    break;
  case CPRIOGET_LWRA_DMA:
    printf(" length-FIFO read/write pointer for DMA");
    break;
  case CPRIOGET_DMA_TRANS:
    printf(" COPPER DMA counter");
    break;
  case CPRIOGET_TS_ENV:
    printf(" COPPER DMA counter control");
    break;
  }

  printf("\n");
  return val;
}
/* ------------------------------------------------------------------ *\
   main
\* ------------------------------------------------------------------ */
int
main(int argc, char **argv)
{
  /* copper and finesse devices */
  int cprfd;

  /* generic variables */
  int ret;

  PROGRAM = argv[0];

  /* COPPER open */
  if ((cprfd = open("/dev/copper/copper_ctl", O_RDONLY)) < 0) {
    printf("%s: can't open /dev/copper/copper_ctl, %s\n",
	   PROGRAM, strerror(errno));
    exit(0);
  }

  getcopper(cprfd, CPRIOGET_FF_STA, "FF_STA");
  getcopper(cprfd, CPRIOGET_CONF_WA_AE, "CONF_WA_AE");
  getcopper(cprfd, CPRIOGET_CONF_WB_AE, "CONF_WB_AE");
  getcopper(cprfd, CPRIOGET_CONF_WC_AE, "CONF_WC_AE");
  getcopper(cprfd, CPRIOGET_CONF_WD_AE, "CONF_WD_AE");
  getcopper(cprfd, CPRIOGET_CONF_WA_FF, "CONF_WA_FF");
  getcopper(cprfd, CPRIOGET_CONF_WB_FF, "CONF_WB_FF");
  getcopper(cprfd, CPRIOGET_CONF_WC_FF, "CONF_WC_FF");
  getcopper(cprfd, CPRIOGET_CONF_WD_FF, "CONF_WD_FF");
  getcopper(cprfd, CPRIOGET_CONF_WA_AF, "CONF_WA_AF");
  getcopper(cprfd, CPRIOGET_CONF_WB_AF, "CONF_WB_AF");
  getcopper(cprfd, CPRIOGET_CONF_WC_AF, "CONF_WC_AF");
  getcopper(cprfd, CPRIOGET_CONF_WD_AF, "CONF_WD_AF");
  getcopper(cprfd, CPRIOGET_FF_RST, "FF_RST");
  getcopper(cprfd, CPRIOGET_FINESSE_STA, "FINESSE_STA");
  getcopper(cprfd, CPRIOGET_ALMFULL_ENB, "ALMFULL_ENB");
  getcopper(cprfd, CPRIOGET_VERSION, "VERSION");
  getcopper(cprfd, CPRIOGET_TRGCM, "TRGCM");
  getcopper(cprfd, CPRIOGET_TRG_LENG, "TRG_LENG");
  getcopper(cprfd, CPRIOGET_LEF_READSEL, "LEF_READSEL");
  getcopper(cprfd, CPRIOGET_LEF_TOTAL, "LEF_TOTAL");
  getcopper(cprfd, CPRIOGET_LEF_AB, "LEF_AB");
  getcopper(cprfd, CPRIOGET_LEF_CD, "LEF_CD");
  getcopper(cprfd, CPRIOGET_LEF_STA, "LEF_STA");
  getcopper(cprfd, CPRIOGET_LEF_WA_FF, "LEF_WA_FF");
  getcopper(cprfd, CPRIOGET_LEF_WB_FF, "LEF_WB_FF");
  getcopper(cprfd, CPRIOGET_LEF_WC_FF, "LEF_WC_FF");
  getcopper(cprfd, CPRIOGET_LEF_WD_FF, "LEF_WD_FF");
  getcopper(cprfd, CPRIOGET_LEF_WA_AF, "LEF_WA_AF");
  getcopper(cprfd, CPRIOGET_LEF_WB_AF, "LEF_WB_AF");
  getcopper(cprfd, CPRIOGET_LEF_WC_AF, "LEF_WC_AF");
  getcopper(cprfd, CPRIOGET_LEF_WD_AF, "LEF_WD_AF");
  getcopper(cprfd, CPRIOGET_INT_STA, "INT_STA");
  getcopper(cprfd, CPRIOGET_INT_MASK, "INT_MASK");
  getcopper(cprfd, CPRIOGET_FF_RW, "FF_RW");
  getcopper(cprfd, CPRIOGET_INT_FACTOR, "INT_FACTOR");

  /*
    20131009: below doesn't work
    20140705: below (single FIFO read/write probably doesn't make sense
    
    getcopper(cprfd, CPRIOGET_FF_WA, "FF_WA");
    getcopper(cprfd, CPRIOGET_FF_WB, "FF_WB");
    getcopper(cprfd, CPRIOGET_FF_WC, "FF_WC");
    getcopper(cprfd, CPRIOGET_FF_WD, "FF_WD");
    getcopper(cprfd, CPRIOGET_FF_RA, "FF_RA");
    getcopper(cprfd, CPRIOGET_FF_RB, "FF_RB");
    getcopper(cprfd, CPRIOGET_FF_RC, "FF_RC");
    getcopper(cprfd, CPRIOGET_FF_RD, "FF_RD");
  */

  getcopper(cprfd, CPRIOGET_EWRP_WA, "EWRP_WA");
  getcopper(cprfd, CPRIOGET_EWRP_WB, "EWRP_WB");
  getcopper(cprfd, CPRIOGET_EWRP_WC, "EWRP_WC");
  getcopper(cprfd, CPRIOGET_EWRP_WD, "EWRP_WD");
  
  getcopper(cprfd, CPRIOGET_WEA_COUNTER, "WEA_COUNTER");
  getcopper(cprfd, CPRIOGET_WEB_COUNTER, "WEB_COUNTER");
  getcopper(cprfd, CPRIOGET_WEC_COUNTER, "WEC_COUNTER");
  getcopper(cprfd, CPRIOGET_WED_COUNTER, "WED_COUNTER");
  
  getcopper(cprfd, CPRIOGET_LWRA_DMA, "LWRA_DMA");
  getcopper(cprfd, CPRIOGET_LWRB_DMA, "LWRB_DMA");
  getcopper(cprfd, CPRIOGET_LWRC_DMA, "LWRC_DMA");
  getcopper(cprfd, CPRIOGET_LWRD_DMA, "LWRD_DMA");

  getcopper(cprfd, CPRIOGET_DMA_TRANS, "DMA_TRANS");
  getcopper(cprfd, CPRIOGET_TS_ENV, "TS_ENV");
  
  return 0;
}

