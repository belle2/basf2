#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "ecl_udp_lib.h"

int col_reg_io(int argn, char**argv)
{
  int rc;
  unsigned int reg_num, reg_wdata=0xCCCC;
  unsigned short int reg_data;
  int command_line_ok = 1;

  char default_ip[]="192.168.1.12";
  char *ip_addr = default_ip;

  int write_cmd = 0;

  switch(argn)
    {
    case 3: // col_reg_io r reg_num
      if( *(argv[1])!='r' || sscanf(argv[2],"%x",&reg_num) != 1 )
	command_line_ok = 0;
      break;
    case 4: // col_reg_io w reg_num reg_data
            // col_reg_io IP_ADDR r reg_num OR
            
      if( *(argv[1])=='w' )
	{
	  write_cmd = 1;
	  if( sscanf(argv[2],"%x",&reg_num) != 1 || sscanf(argv[3],"%x",&reg_wdata) != 1 )
	    command_line_ok = 0;
	}
      else if( *(argv[2])=='r' )
	{
	  ip_addr = argv[1];
	  if( sscanf(argv[3],"%x",&reg_num) != 1 )
	    command_line_ok = 0;
	}
      else
	command_line_ok = 0;
      break;
    case 5: // col_reg_io IP_ADDR w reg_num reg_data
      ip_addr = argv[1];
      write_cmd = 1;
      if( *(argv[2])!='w' || sscanf(argv[3],"%x",&reg_num) != 1 || sscanf(argv[4],"%x",&reg_wdata) != 1 )
	command_line_ok = 0;
      break;
    default:
      command_line_ok = 0;
      break;
    }

  if( command_line_ok == 0 )
    {
      printf("Syntax: col_reg_io [IP_ADDR] r reg_num   OR   col_reg_io [IP_ADDR] w reg_num reg_data\n");
      return 1;
    }

  //  printf("%s %d %x %x\n\n", ip_addr, write_cmd, reg_num, reg_wdata);


  if((rc=ecl_udp_init_ip(ip_addr))!=0)
    {
      printf("Can not init network (%d,%d)\n", rc, errno);
      return 2;
    }
  if(write_cmd==0)
    {
      if((rc=ecl_udp_read_reg(reg_num, &reg_data))!=0)
	{
	  printf("ecl_udp_read_reg() fault (%d,%d)\n", rc, errno);
	  return 3;
	}
      printf("[%04X] => %04X\n", reg_num, reg_data);
    }
  else
    {
      if((rc=ecl_udp_write_reg(reg_num, reg_wdata))!=0)
        {
          printf("ecl_udp_write_reg() fault (%d,%d)\n", rc, errno);
          return 3;
        }
      printf("[%04X] <= %04X\n", reg_num, reg_wdata);
    }
  
  return 0;
}

#ifdef EXE_MODE
int main(int argn, char**argv)
{
  return col_reg_io(argn, argv);
}
#endif
