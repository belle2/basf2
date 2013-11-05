#include "mgt.h"
#include "mgt_control.h"
#include "mgt_register.h"

#include <stdlib.h>
#include <stdio.h>

Mgt_t* mgt_boot(int slot, const char* path, int* board_type,
		int* firmware, int* hardware) {
  if ( slot < 0 ) return 0;
  Mgt_t* mgt = 0;
  if ( (mgt = mgt_open(slot, MGT_RDWR)) == 0 ) return 0;
  *board_type = mgt_read(mgt, FEE_TYPE);
  *firmware = mgt_read(mgt, 0x71);
  *hardware = mgt_read(mgt, 0x70);
  switch (*board_type) {
  case 0x01:
    printf("The type of FEE readout board which is linked to HSLB:%c is CDC\n",'a'+slot);
    break;
  case 0x02:
    printf("The type of FEE readout board which is linked to HSLB:%c is ECL\n",'a'+slot);
    break;
  default:
    printf("error type number : %x\n", *board_type);
  }
  printf("Firmware ver.: %d\n", *firmware);
  printf("Hardware ver.: %d\n", *hardware);
  return mgt;
  /*
  printf("boot FPGA\n");
  if ( mgt_boot_fpga(mgt, path, 0, 0, M012_SELECTMAP) >= 0 ) {
    printf("boot FPGA: done\n");
    return mgt;
  } else { 
    printf("boot FPGA: failed\n");
    mgt_close(mgt);
    return 0;
  }
  */
}

