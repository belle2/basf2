#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "ecl_udp_lib.h"
#include "ecl_collector.h"
#include "ecl_collector_lib.h"

int sh_reg_io(const char* ip_addr, const char* opt, int sh_num, int reg_num, int reg_wdata)
{
  int rc;
  unsigned short int reg_data;
  unsigned short int read_ok_mask;
  int sh_reg_data[12];
  unsigned int sh_mask;
  int sh_num_i, sh_mask_i, read_ok_mask_i;

  sh_num--;

  if((rc = ecl_udp_init_ip(ip_addr))!=0) {
    printf("Can not init network (%d,%d)\n", rc, errno);
    return 2;
  }
  sh_mask = (sh_num == 15)? 0xFFF :1 << sh_num;

  if((rc = ecl_udp_read_reg(_ECL_COLLECTOR_REG_DL_LOCKED, &reg_data)) != 0) {
    printf("ecl_udp_read_reg() fault (%d,%d)\n", rc, errno);
    return 3;
  }

  if((sh_mask & reg_data) != sh_mask)
    printf("WARNING: RCLK is not locked!: DL_LOCKED_REG=%04X, expecting: %04X\n", reg_data, sh_mask);
  
  if((rc = ecl_udp_read_reg(_ECL_COLLECTOR_REG_DL_READ_PEND, &reg_data)) != 0) {
    printf("ecl_udp_read_reg() fault (%d,%d)\n", rc, errno);
    return 4;
  }

  if((sh_mask & reg_data) != 0) {
    printf("WARNING: deser buffer is not empty: DL_READ_PEND=%04X, shaper mask=%04X Flushing...\n", reg_data, sh_mask);
    
    for(sh_num_i = 0, sh_mask_i = sh_mask; sh_num_i < 12; sh_num_i++, sh_mask_i >>= 1) {
      if((sh_mask_i&1)!=0) {
	if( (rc = ecl_udp_read_reg(_ECL_COLLECTOR_REG_DL_NEXT_PCT_BASE + sh_num_i, &reg_data)) != 0) {
	  printf("ecl_udp_read_reg() fault (%d,%d)\n", rc, errno);
	  return 5;
	}
	if((rc = ecl_udp_write_reg(_ECL_COLLECTOR_REG_DL_REF_PCT_BASE + sh_num_i, reg_data)) != 0) {
	  printf("ecl_udp_write_reg() fault (%d,%d)\n", rc, errno);
	  return 6;
	}
      }
    }
    if((rc = ecl_udp_read_reg(_ECL_COLLECTOR_REG_DL_READ_PEND, &reg_data)) != 0) {
      printf("ecl_udp_read_reg() fault (%d,%d)\n", rc, errno);
      return 7;
    }
    if((sh_mask & reg_data) != 0) {
      printf("ERROR: deser buffer still is not empty and flushing doesn't help: DL_READ_PEND=%04X, shaper mask=%04X\n", reg_data, sh_mask);
      return 8;
    }
  }

  if(strcmp(opt, "r") == 0) {
    sh_read_reg(sh_num, reg_num, sh_reg_data, &read_ok_mask);
    for(sh_num_i = 0, sh_mask_i = sh_mask, read_ok_mask_i = read_ok_mask; 
	sh_num_i < 12; sh_num_i++, sh_mask_i >>= 1, read_ok_mask_i >>= 1 ) {
      if((sh_mask_i & 1) != 0) {
	if((read_ok_mask_i & 1) != 0) {
	  printf("ShaperDSP#%d[%04X] => %04X\n", sh_num_i+1, reg_num, sh_reg_data[sh_num_i]);
	} else {
	  printf("ShaperDSP#%d[%04X] ...\n", sh_num_i+1, reg_num);
	}
      }
    }
  } else if(strcmp(opt, "w") == 0) {
    sh_write_reg(sh_num, reg_num, reg_wdata);
    for(sh_num_i = 0, sh_mask_i = sh_mask; sh_num_i < 12; sh_num_i++, sh_mask_i >>= 1) {
      if((sh_mask_i & 1) != 0) {
	printf("ShaperDSP#%d[%04X] <= %04X\n", sh_num_i+1, reg_num, reg_wdata);
      }
    }
  }
  return 0;
}

