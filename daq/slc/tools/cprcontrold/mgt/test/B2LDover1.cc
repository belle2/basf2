#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "mgt.h"
#include "mgt_register.h"
#include "mgt_control.h"

const int FirmwareVersion = 0x000a;
const int HardwareVersion = 0x000a;

char* ARGV0  = 0;
char* DEVICE = 0;
unsigned int adr, adr_tem;

static struct { const char* name; int adrs; } regs[] = {
  { "trigger",   CONTROL },
  { "trghold",   CONTROL },
  { "checkerr",  CONTROL },
  { "checkfee",  CONTROL },
  { "link",      CONTROL },
  { "linkrst",   CONTROL },
  { "verbose",   CONTROL },
  { "simple",    CONTROL },
  { "window",    WINDOW  },
  { "delay",     DELAY   },
  { "adcth" ,    ADCTH   },
  { "pdstl",     PDSTL   },
  { "tdcth",     TDCTH   },
  { "pdstlall",  PDSTL   }
};

/* ---------------------------------------------------------------------- *\
   usage
\* ---------------------------------------------------------------------- */
static void usage()
{
  int m;
  printf("usage: %s -(a,b,c,d)  <parameter(char)>\n", ARGV0);
  printf("valid parameter options : ");
  for (m = 0 ; m < sizeof(regs) / sizeof(regs[0]); m++)
    printf("  %s  ", regs[m].name);
  printf("\n");
}

int main(int argc, char** argv)
{
  Mgt_t* mgt[4] = { NULL, NULL, NULL, NULL };
  int use[4];
  int readback[4];
  int i, j, ii;
  FILE* fp;
  int ch;
  int tem;
  int k = 0, l;
  int val, val_tem;
  int feetype, feehwv, feefwv;
  time_t   lt;
  int err_check, err_ini;
  if (argc < 2) {
    usage();
    return 1;
  }
  readback[0] = readback[1] = readback[2] = readback [3] = 0;
  use[0] = use[1] = use[2] = use[3] = 0;
  ARGV0 = argv[0];
  for (i = 1; argv[1][i]; i++) {
    DEVICE = 0;
    if (! strchr("abcd", argv[1][i])) {
      argc = -1;
      break;
    }

    j = argv[1][i] - 'a';
    if (use[j]) {
      fprintf(stderr, "HSLB %c is specified twice\n", 'a' + j);
      return 1;
    }
    use[j] = 1;
  }
  argv++, argc--;

  if (argc < 2 || (use[0] + use[1] + use[2] + use[3]) == 0) {
    usage();
    return 1;
  }
  /* check the device  */
  for (i = 0; i < 4; i++) {
    if (!use[i]) continue;
    if (mgt[i] == NULL) {
      if ((mgt[i] = mgt_open(i, MGT_RDWR)) == 0) {
        fprintf(stderr, "%s: cannot open %s: %s\n", ARGV0, DEVICE, strerror(errno));
        return 1;
      }
      if ((tem =  mgt_read(mgt[i], HSLB_FMV))  != FirmwareVersion) {
        fprintf(stderr, "HLSB Firmware version %.4x is not suitable for this applicaton\n", tem);
        return 1;
      }
      if ((tem =  mgt_read(mgt[i], HSLB_HWV))  != HardwareVersion) {
        fprintf(stderr, "HLSB Hardware version %.4x is not suitable for this applicaton\n", tem);
        return 1;
      }
    }
  }
  if (!strcmp(argv[1] , "link")) {
    for (i = 0 ; i < 4 ; i ++) {
      if (!use[i]) continue;
      mgt_execute(mgt[i],  CTL_LINK);
    }
    return 0;
  } else if (!strcmp(argv[1] , "linkrst")) {
    for (i = 0 ; i < 4 ; i ++) {
      if (!use[i]) continue;
      mgt_execute(mgt[i],  CTL_RESET_LINK);
    }
  } else if (!strcmp(argv[1] , "checkerr")) {
    for (i = 0 ; i < 4 ; i ++) {
      if (!use[i]) continue;
    }
  } else if (!strcmp(argv[1] , "trghold")) {
    for (i = 0 ; i < 4 ; i ++) {
      if (!use[i]) continue;
      mgt_execute(mgt[i], CTL_HOLD_TRG);
    }
  } else if (!strcmp(argv[1] , "realtrg")) {
    for (i = 0 ; i < 4 ; i ++) {
      if (!use[i]) continue;
      mgt_execute(mgt[i], CTL_TRG_REAL);
    }
  } else if (!strcmp(argv[1] , "simtrg")) {
    for (i = 0 ; i < 4 ; i ++) {
      if (!use[i]) continue;
      mgt_execute(mgt[i], CTL_TRG_SIM);
    }
  } else if (!strcmp(argv[1] , "trigger")) {
    for (i = 0 ; i < 4 ; i ++) {
      if (!use[i]) continue;
      mgt_execute(mgt[i], CTL_TRG_NORMAL);
    }
  } else if (!strcmp(argv[1] , "verbose")) {
    for (i = 0 ; i < 4 ; i ++) {
      if (!use[i]) continue;
      mgt_execute(mgt[i], CTL_VERBOSE);
    }
  } else if (!strcmp(argv[1] , "simple")) {
    for (i = 0 ; i < 4 ; i ++) {
      if (!use[i]) continue;
      mgt_execute(mgt[i], CTL_SIMPLE);
    }
  } else if (argv[1][1] == 'p' & argv[1][0] == '-') {
    argc--;
    argv++;
    for (i = 0 ; i < 4 ; i ++) {
      if (!use[i]) continue;
      mgt_write(mgt[i], CONTROL, 0x05); //reset address fifo
    }
    while (argc > 1) {
      if (argc < 3) {
        usage();
        return 1;
      }
      for (i = 0; i < sizeof(regs) / sizeof(regs[0]); i++) {
        if (!strcmp(regs[i].name, argv[1])) {
          adr = regs[i].adrs;
          break;
        }
      }
      if (i == sizeof(regs) / sizeof(regs[0])) {
        usage();
        return 1;
      }
      if (!strcmp("pdstl", argv[1])) {  //configure pedistel
        if (argc < 4) {
          usage();
          return 1;
        }
        val = strtoul(argv[2], 0 , 0);
        adr = adr + 2 * (val - 1);
        val = strtoul(argv[3], 0 , 0);
        for (i = 0; i < 4; i++) {
          if (!use[i]) continue;
          mgt_write(mgt[i], adr, val);
          printf("written %.2x to address %x on HSLB:%c\n", val, adr, 'a' + i);
          adr_tem = adr + 1;
          val_tem = val >> 8;
          mgt_write(mgt[i], adr_tem, val_tem);
          printf("written %.2x to address %x on HSLB:%c\n", val_tem, adr_tem, 'a' + i);
        }
        argv += 3;
        argc -= 3;
      } else  if (!strcmp("pdstlall", argv[1])) { //configure pedistel
        if (argc < 3) {
          usage();
          return 1;
        }
        val = strtoul(argv[2], 0 , 0);
        for (ii = 0 ; ii < 48; ii++) {
          for (i = 0; i < 4; i++) {
            if (!use[i]) continue;
            mgt_write(mgt[i], adr, val);
            printf("written %.2x to address %x on HSLB:%c\n", val, adr, 'a' + i);
            adr_tem = adr + 1;
            val_tem = val >> 8;
            mgt_write(mgt[i], adr_tem, val_tem);
            printf("written %.2x to address %x on HSLB:%c\n", val_tem, adr_tem, 'a' + i);
          }
          adr = adr + 2;
        }
        argv += 3;
        argc -= 3;
      } else if (!strcmp("adcth", argv[1])) { //configure threhold
        val = strtoul(argv[2], 0, 0);
        for (i = 0; i < 4; i++) {
          if (!use[i]) continue;
          mgt_write(mgt[i], adr, val);
          printf("written %.2x to address %x on HSLB:%c\n", val, adr, 'a' + i);
          adr_tem = adr + 1;
          val_tem = val >> 8;
          mgt_write(mgt[i], adr_tem, val_tem);
          printf("written %.2x to address %x on HSLB:%c\n", val_tem, adr_tem, 'a' + i);
        }
        argv += 2;
        argc -= 2;
      } else if (!strcmp("tdcth", argv[1])) { //configure threhold
        val = strtoul(argv[2], 0, 0);
        for (i = 0; i < 4; i++) {
          if (!use[i]) continue;
          mgt_write(mgt[i], adr, val);
          printf("written %.2x to address %x on HSLB:%c\n", val, adr, 'a' + i);
          adr_tem = adr + 1;
          val_tem = val >> 8;
          mgt_write(mgt[i], adr_tem, val_tem);
          printf("written %.2x to address %x on HSLB:%c\n", val_tem, adr_tem, 'a' + i);
        }
        argv += 2;
        argc -= 2;
      } else {
        val = strtoul(argv[2], 0, 0);
        for (i = 0; i < 4; i++) {
          if (!use[i]) continue;
          mgt_write(mgt[i], adr, val);
          printf("written %.2x to address %x on HSLB:%c\n", val, adr, 'a' + i);
        }
        argv += 2;
        argc -= 2;
      }
    }
    for (i = 0; i < 4; i++) {
      if (!use[i]) continue;
      printf("written %.2x to address %x on HSLB:%c\n", 0x0a, CONTROL, 'a' + i);
      mgt_write(mgt[i], CONTROL, 0x0a);
    }
  } else if (!strcmp(argv[1] , "checkfee")) {
    for (i = 0; i < 4; i++) {
      if (!use[i]) continue;
      mgt_check_FEE(mgt[i]);
    }
  } else {
    usage();
    return 1;
  }
  return 0;
}

