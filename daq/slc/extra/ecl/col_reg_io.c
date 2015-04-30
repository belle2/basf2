#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "ecl_udp_lib.h"

int col_reg_io(const char* ip_addr, const char* opt, 
	       int reg_num, int reg_wdata, 
	       unsigned short* reg_data, char* msg)
{
  int rc;
  if((rc = ecl_udp_init_ip(ip_addr)) != 0) {
    sprintf(msg, "Can not init network (%d,%d)\n", rc, errno);
    ecl_udp_close();
    return 2;
  }

  if(strcmp(opt, "r") == 0) {
    if((rc=ecl_udp_read_reg(reg_num, reg_data)) != 0) {
      sprintf(msg, "ecl_udp_read_reg() fault (%d,%d)\n", rc, errno);
      ecl_udp_close();
      return 3;
    }
    printf("[%04X] => %04X\n", reg_num, *reg_data);
  } else if(strcmp(opt, "w") == 0) {
    if((rc=ecl_udp_write_reg(reg_num, reg_wdata)) != 0) {
      sprintf(msg, "ecl_udp_write_reg() fault (%d,%d)\n", rc, errno);
      ecl_udp_close();
      return 4;
    }
    sprintf(msg, "[%04X] <= %04X\n", reg_num, reg_wdata);
  }
  ecl_udp_close();
  
  return 0;
}

