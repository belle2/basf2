#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>  
#include <arpa/inet.h>

#include "ecl_udp_lib.h"
#include "ecl_collector.h"
#include "ecl_collector_lib.h"

#define ECL_TCP_STATUS_PORT 6001

int sh_read_dsp_data(int sh_num, int sh_dsp_data_type, unsigned short int *buf)
{
  int rc;
  unsigned short int reg_data;
  int mem_addr;

  int i;

  unsigned short int read_buf[36];

  if((rc=ecl_udp_write_reg( _ECL_COLLECTOR_REG_MEM_IO_SH_NUM, sh_num))!=0)
    return 1;

  if((rc=ecl_udp_write_reg( _ECL_COLLECTOR_REG_DSP_MEM_TYPE, sh_dsp_data_type))!=0)
    return 2;

  if((rc=ecl_udp_write_reg( _ECL_COLLECTOR_REG_COMMAND, _ECL_COLLECTOR_REG_COMMAND_READ_DSP_MEM))!=0)
    return 3;

  if((rc=ecl_udp_read_reg(_ECL_COLLECTOR_REG_DL_REF_PCT_BASE+sh_num, &reg_data))!=0)
    return 4;

  mem_addr = _ECL_COLLECTOR_MEM_DSR_BUFFER_BASE +_ECL_COLLECTOR_MEM_DSP_BUFFER_SH_OFFSET*sh_num + 2*reg_data;

  if( (rc=ecl_udp_read_mem( mem_addr, 36, read_buf))!=0)
    return 5;

  printf("ShaperDSP#%d, mem_addr=%08X:\n%04X.%04X\n", sh_num+1, mem_addr, read_buf[1], read_buf[0]);
  printf("%04X.%04X\n", read_buf[3], read_buf[2]);
  printf("%04X.%04X\n", read_buf[5], read_buf[4]);
  printf("%04X.%04X\n", read_buf[7], read_buf[6]);
  printf("...\n");
  printf("%04X.%04X\n", read_buf[35], read_buf[34]);
  
  if(read_buf[0]!=0x12 || read_buf[1] != 4 || read_buf[2] != sh_dsp_data_type)
    return 6;

  for(i=0;i<16;i++)
    buf[i]=read_buf[4+(i<<1)];

  if( (rc=ecl_udp_read_reg(_ECL_COLLECTOR_REG_DL_NEXT_PCT_BASE+sh_num, &reg_data))!=0)
    return 7;
  if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_DL_REF_PCT_BASE+sh_num, reg_data))!=0)
    return 8;

  return 0;
}

int sh_write_dsp_data(int sh_num, int sh_dsp_data_type, unsigned short int *buf)
{
  int rc;

  int i;

  unsigned int write_buf[16];

  if((rc=ecl_udp_write_reg( _ECL_COLLECTOR_REG_MEM_IO_SH_NUM, sh_num))!=0)
    return 1;

  if((rc=ecl_udp_write_reg( _ECL_COLLECTOR_REG_DSP_MEM_TYPE, sh_dsp_data_type))!=0)
    return 2;

  for(i=0;i<16;i++)
    write_buf[i]=buf[i];

  if((rc=ecl_udp_write_mem( _ECL_COLLECTOR_MEM_BUF_MEM_IO_BASE + _ECL_COLLECTOR_MEM_BUF_MEM_IO_SH_OFFSET*sh_num, 32, (unsigned short *)write_buf))!=0)
    return 3;

  if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_COMMAND, _ECL_COLLECTOR_REG_COMMAND_WRITE_DSP_MEM))!=0)
    return 4;

  return 0;
}

int sh_write_sdram_data(int sh_num, int chip, int bank, int row, int col, unsigned short int *buf)
{
  // chip 0,1 or 2 (both)

  int rc;
  if((rc=ecl_udp_write_reg( _ECL_COLLECTOR_REG_MEM_IO_SH_NUM, sh_num))!=0)
    return 1;

  if((rc=ecl_udp_write_reg( _ECL_COLLECTOR_REG_SDRAM_ROW, row))!=0)
    return 2;

  if((rc=ecl_udp_write_reg( _ECL_COLLECTOR_REG_SDRAM_COL_HIGH, col>>5))!=0)
    return 2;

  if((rc=ecl_udp_write_mem( _ECL_COLLECTOR_MEM_BUF_MEM_IO_BASE + _ECL_COLLECTOR_MEM_BUF_MEM_IO_SH_OFFSET*sh_num, 32, buf))!=0)
    return 3;

  if(chip==0 || chip==2)
    {

      if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_SDRAM_CHIP_BANK, bank))!=0)
	return 4;
    
      if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_COMMAND, _ECL_COLLECTOR_REG_COMMAND_WRITE_SDRAM))!=0)
	return 5;      
    }

  if(chip==1 || chip==2)
    {

      if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_SDRAM_CHIP_BANK, 4 | bank))!=0)
	return 6;
    
      if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_COMMAND, _ECL_COLLECTOR_REG_COMMAND_WRITE_SDRAM))!=0)
	return 7;      
    }

  return 0;
}



int sh_read_reg(int sh_num, int sh_reg_num, int* sh_reg_data, unsigned short int *read_ok_mask)
{
  int rc;
  unsigned short int reg_data;
  //  unsigned short int pct_offset;
  unsigned short int read_pend;
  int sh_mask;
  int sh_num_i, sh_mask_i;

  unsigned short int buf[10];

  int mem_addr;

  if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_DL_REG_RW_SH_NUM, sh_num))!=0)
    {
      printf("ecl_udp_write_reg() fault (%d,%d)\n", rc, errno);
      return 1;
    }

  if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_DL_REG_RW_REG_NUM, sh_reg_num))!=0)
    {
      printf("ecl_udp_write_reg() fault (%d,%d)\n", rc, errno);
      return 2;
    }

  if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_DL_REG_RW_CMD, 1<<_ECL_COLLECTOR_REG_DL_REG_RW_CMD_READ_POS))!=0)
    {
      printf("ecl_udp_write_reg() fault (%d,%d)\n", rc, errno);
      return 3;
    }
  
  if((rc=ecl_udp_read_reg(_ECL_COLLECTOR_REG_DL_READ_PEND, &read_pend))!=0)
    {
      printf("ecl_udp_read_reg() fault (%d,%d)\n", rc, errno);
      return 4;
    }

  sh_mask=sh_num==15?0xFFF:1<<sh_num;
  
  *read_ok_mask = sh_mask&read_pend;

  if( *read_ok_mask != sh_mask)
      printf("WARNING read register timeout: COL_REG_READ_PEND=%03X, expecting: %03X\n", read_pend, sh_mask);

  for(sh_num_i=0, sh_mask_i=*read_ok_mask; sh_num_i<12; sh_num_i++, sh_mask_i>>=1)
    {
      if((sh_mask_i&1)==0)
	continue;
      if((rc=ecl_udp_read_reg(_ECL_COLLECTOR_REG_DL_REF_PCT_BASE+sh_num_i, &reg_data))!=0)
	{
	  printf("ecl_udp_read_reg() fault (%d,%d)\n", rc, errno);
	  return 6;
	}

      mem_addr = _ECL_COLLECTOR_MEM_DSR_BUFFER_BASE +_ECL_COLLECTOR_MEM_DSP_BUFFER_SH_OFFSET*sh_num_i + 2*reg_data;

      if( (rc=ecl_udp_read_mem( mem_addr, 4, buf))!=0)
	{
	  printf("ecl_udp_read_mem() fault (%d,%d)\n", rc, errno);
	  return 7;
	}

      ecl_udp_lib_debug?printf("ShaperDSP#%d, mem_addr=%08X: %04X %04X %04X %04X\n", sh_num_i+1, mem_addr, buf[0], buf[1], buf[2], buf[3]):0;
      
      if(buf[0]!=2 || (buf[1]&0xFF)!=0 || buf[2] != sh_reg_num)
	{
	  printf("ERROR: ShaperDSP#%d - unexpected response at mem_addr=%08X: %04X %04X %04X %04X\n", sh_num_i+1, mem_addr, buf[0], buf[1], buf[2], buf[3]);
	  return 8;
	}      
      sh_reg_data[sh_num_i]=buf[3];

      if( (rc=ecl_udp_read_reg(_ECL_COLLECTOR_REG_DL_NEXT_PCT_BASE+sh_num_i, &reg_data))!=0)
	{
	  printf("ecl_udp_read_reg() fault (%d,%d)\n", rc, errno);
	  return 9;
	}
      if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_DL_REF_PCT_BASE+sh_num_i, reg_data))!=0)
	{
	  printf("ecl_udp_write_reg() fault (%d,%d)\n", rc, errno);
	  return 10;
	}
    }
  return 0;
}

int sh_write_reg(int sh_num, int sh_reg_num, int sh_reg_data)
{
  int rc;

  if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_DL_REG_RW_SH_NUM, sh_num))!=0)
    {
      printf("ecl_udp_write_reg() fault (%d,%d)\n", rc, errno);
      return 1;
    }

  if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_DL_REG_RW_REG_NUM, sh_reg_num))!=0)
    {
      printf("ecl_udp_write_reg() fault (%d,%d)\n", rc, errno);
      return 2;
    }

  if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_DL_REG_RW_REG_WDATA, sh_reg_data))!=0)
    {
      printf("ecl_udp_write_reg() fault (%d,%d)\n", rc, errno);
      return 2;
    }

  if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_DL_REG_RW_CMD, 1<<_ECL_COLLECTOR_REG_DL_REG_RW_CMD_WRITE_POS))!=0)
    {
      printf("ecl_udp_write_reg() fault (%d,%d)\n", rc, errno);
      return 3;
    }
  return 0;
}

int sh_get_event(int sh_num, int *event_data)
{
  int rc;
  unsigned short int reg_data;

  int i, tmp;
  int weight,dweight;

  int mem_addr;

  int pct_len, length_to_read, length_read, offset;

  unsigned short int *buf = (unsigned short int*)event_data;

  if((rc=ecl_udp_read_reg(_ECL_COLLECTOR_REG_DL_REF_PCT_BASE+sh_num, &reg_data))!=0)
    {
      printf("ecl_udp_read_reg() fault (%d,%d)\n", rc, errno);
      return 6;
    }

  mem_addr = _ECL_COLLECTOR_MEM_DSR_BUFFER_BASE +_ECL_COLLECTOR_MEM_DSP_BUFFER_SH_OFFSET*sh_num + 2*reg_data;

  if( (rc=ecl_udp_read_mem( mem_addr, 8, buf))!=0)
    {
      printf("ecl_udp_read_mem() fault (%d,%d)\n", rc, errno);
      return 7;
    }

  ecl_udp_lib_debug?printf("ShaperDSP#%d, mem_addr=%08X: %04X.%04X %04X.%04X %04X.%04X %04X.%04X\n", sh_num+1, mem_addr, buf[1], buf[0], buf[3], buf[2], buf[5], buf[4], buf[7], buf[6]):0;
  //  printf("ShaperDSP#%d, mem_addr=%08X: %04X.%04X %04X.%04X %04X.%04X %04X.%04X\n", sh_num+1, mem_addr, buf[1], buf[0], buf[3], buf[2], buf[5], buf[4], buf[7], buf[6]);

  for(i=0,weight=0,tmp=buf[6];i<16;i++,tmp>>=1)  // ADC normal
    weight+=tmp&0x01;

  for(i=0,tmp=buf[7];i<16;i++,tmp>>=1)           // ADC high
    weight+=tmp&0x01;

  pct_len=(buf[3]&0xFF)*weight;

  for(i=0,dweight=0,tmp=buf[5];i<16;i++,tmp>>=1)   // DSP
    dweight+=tmp&0x01;

  pct_len += dweight;

  if(((buf[1]&0xFF)!=0x10) || weight!=((buf[3]>>8)&0xFF) || dweight!=((buf[2]>>8)&0xFF) || ((buf[0]-pct_len)!=4) )
    {

      ecl_udp_lib_debug?printf("Event packet broken!\n"):0;
      return 8;
    }

  for(offset=8, length_read=0; length_read<pct_len;length_read+=length_to_read)
    {
      length_to_read=pct_len-length_read;
      length_to_read=length_to_read>256?256:length_to_read;

      if( (rc=ecl_udp_read_mem( mem_addr+offset, length_to_read<<1, buf+offset))!=0)
	{
	  printf("ecl_udp_read_mem() fault (%d,%d)\n", rc, errno);
	  return 9;
	}
      //      printf(".");
      offset += length_to_read<<1;
    }
  //      printf("\n");

  // Moving the pointer
  if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_DL_REF_PCT_BASE+sh_num, reg_data+pct_len+4))!=0)
    {
      printf("ecl_udp_write_reg() fault (%d,%d)\n", rc, errno);
      return 6;
    }


  return 0;
}


int sh_get_mem_data(int sh_num, int *mem_data)
{
  int rc;
  unsigned short int reg_data;

  int mem_addr;


  unsigned short int *buf = (unsigned short int*)mem_data;

  if((rc=ecl_udp_read_reg(_ECL_COLLECTOR_REG_DL_REF_PCT_BASE+sh_num, &reg_data))!=0)
    {
      printf("ecl_udp_read_reg() fault (%d,%d)\n", rc, errno);
      return 6;
    }

  mem_addr = _ECL_COLLECTOR_MEM_DSR_BUFFER_BASE +_ECL_COLLECTOR_MEM_DSP_BUFFER_SH_OFFSET*sh_num + 2*reg_data;
  
  if( (rc=ecl_udp_read_mem( mem_addr, 36, buf))!=0)
    {
      printf("ecl_udp_read_mem() fault (%d,%d)\n", rc, errno);
      return 7;
    }
  
  ecl_udp_lib_debug?printf("ShaperDSP#%d, mem_addr=%08X: %04X.%04X %04X.%04X %04X.%04X %04X.%04X\n", sh_num+1, mem_addr, buf[1], buf[0], buf[3], buf[2], buf[5], buf[4], buf[7], buf[6]):0;

  if(buf[0]!=18 || buf[1]!=2)
    {
      printf("Wrong SDRAM data format!\n");
      return 8;
    }

  // Moving the pointer
  if((rc=ecl_udp_write_reg(_ECL_COLLECTOR_REG_DL_REF_PCT_BASE+sh_num, reg_data+18))!=0)
    {
      printf("ecl_udp_write_reg() fault (%d,%d)\n", rc, errno);
      return 8;
    }


  return 0;
}

