#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include "ecl_collector_lib.h"

const unsigned int MEM_BIN_ADDR = 0xA7000000;

void w_sh_reg_io(const char* ip,
                 int reg_num, int wdata)
{
  char msg[100];
  memset(msg, 0, 100);
  int ret;
  if ((ret = sh_reg_io(ip, "w", 16, reg_num, wdata, NULL, msg)) != 0) {
    if (msg[0] != 0) {
      printf("error: %d >> %s\n", ret, msg);
      exit(1);
    }
  }
}

void w_col_reg_io(const char* ip,
                  int reg_num, int wdata)
{
  char msg[100];
  memset(msg, 0, 100);
  col_reg_io(ip, "w", reg_num, wdata, NULL, msg);
  if (msg[0] != 0) {
    printf("%s", msg);
    exit(1);
  }
}

void loshp(const char* ip, int col)
{
  const int SHAPER_PROC_MASK = 0xffff;
  const int SHAPER_PROC_NUM  = 0x10;
  const int SHAPER_ADC_MASK  = 0xffff;
  const int SHAPER_ADC_NUM   = 0x10;
  //const int CALIB_RELAY = 0;
  const int TRG2ADC_DATA_END = 0x2D;
  const int SH_RELAY = 0x0000;
  const int ADC_DATA_LEN = 0x40;
  const int ADC1_COMP_VALUE = 0xB6;
  const int ADC2_COMP_VALUE = 0xB6;
  const int ADC3_COMP_VALUE = 0xB6;
  const int ADC4_COMP_VALUE = 0xB6;
  const int ADC5_COMP_VALUE = 0xB6;
  const int ADC6_COMP_VALUE = 0xB6;
  const int ADC7_COMP_VALUE = 0xB6;
  const int ADC8_COMP_VALUE = 0xB6;
  const int ADC9_COMP_VALUE = 0xB6;
  const int ADC10_COMP_VALUE = 0xB6;
  const int ADC11_COMP_VALUE = 0xB6;
  const int ADC12_COMP_VALUE = 0xB6;
  const int ADC13_COMP_VALUE = 0xB6;;
  const int ADC14_COMP_VALUE = 0xB6;
  const int ADC15_COMP_VALUE = 0xB6;
  const int ADC16_COMP_VALUE = 0xB6;
  const int TRBUF = 0xc8;
  const int UTHREAD_AF = 0xc0;
  const int THREAD_AF = 0xD;

  w_sh_reg_io(ip, 0x502, THREAD_AF);
  w_sh_reg_io(ip, 0x500, TRBUF);
  w_sh_reg_io(ip, 0x501, UTHREAD_AF);

  w_sh_reg_io(ip, 0x200, SHAPER_PROC_MASK);
  w_sh_reg_io(ip, 0x210, SHAPER_PROC_NUM);

  w_sh_reg_io(ip, 0x208, SHAPER_ADC_MASK);
  w_sh_reg_io(ip, 0x218, SHAPER_ADC_NUM);

  w_sh_reg_io(ip, 0x220, TRG2ADC_DATA_END);

  w_sh_reg_io(ip, 0x20, ADC1_COMP_VALUE);
  w_sh_reg_io(ip, 0x21, ADC2_COMP_VALUE);
  w_sh_reg_io(ip, 0x22, ADC3_COMP_VALUE);
  w_sh_reg_io(ip, 0x23, ADC4_COMP_VALUE);
  w_sh_reg_io(ip, 0x24, ADC5_COMP_VALUE);
  w_sh_reg_io(ip, 0x25, ADC6_COMP_VALUE);
  w_sh_reg_io(ip, 0x26, ADC7_COMP_VALUE);
  w_sh_reg_io(ip, 0x27, ADC8_COMP_VALUE);
  w_sh_reg_io(ip, 0x28, ADC9_COMP_VALUE);
  w_sh_reg_io(ip, 0x29, ADC10_COMP_VALUE);
  w_sh_reg_io(ip, 0x2A, ADC11_COMP_VALUE);
  w_sh_reg_io(ip, 0x2B, ADC12_COMP_VALUE);
  w_sh_reg_io(ip, 0x2C, ADC13_COMP_VALUE);
  w_sh_reg_io(ip, 0x2D, ADC14_COMP_VALUE);
  w_sh_reg_io(ip, 0x2E, ADC15_COMP_VALUE);
  w_sh_reg_io(ip, 0x2F, ADC16_COMP_VALUE);

  w_sh_reg_io(ip, 0x40, SH_RELAY);

  w_sh_reg_io(ip, 0x223, ADC_DATA_LEN);
  w_sh_reg_io(ip, 0x800, 1);

  // reset DAC
  w_col_reg_io(ip, 0xC000, 1);

  const int POT1  = 0x3f;
  const int POT2  = 0x3f;
  const int POT3  = 0x3f;
  const int POT4  = 0x3f;
  const int POT5  = 0x3f;
  const int POT6  = 0x3f;
  const int POT7  = 0x3f;
  const int POT8  = 0x3f;
  const int POT9  = 0x3f;
  const int POT10 = 0x3f;
  const int POT11 = 0x3f;
  const int POT12 = 0x3f;
  const int POT13 = 0x3f;
  const int POT14 = 0x3f;
  const int POT15 = 0x3f;
  const int POT16 = 0x3f;

  w_sh_reg_io(ip, 0x30, POT1);
  w_sh_reg_io(ip, 0x31, 0x10);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT2);
  w_sh_reg_io(ip, 0x31, 0x11);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT3);
  w_sh_reg_io(ip, 0x31, 0x12);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT4);
  w_sh_reg_io(ip, 0x31, 0x13);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT5);
  w_sh_reg_io(ip, 0x31, 0x14);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT6);
  w_sh_reg_io(ip, 0x31 , 0x15);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT7);
  w_sh_reg_io(ip, 0x31, 0x16);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT8);
  w_sh_reg_io(ip, 0x31, 0x17);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT9);
  w_sh_reg_io(ip, 0x31, 0x18);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT10);
  w_sh_reg_io(ip, 0x31, 0x19);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT11);
  w_sh_reg_io(ip, 0x31, 0x1A);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT12);
  w_sh_reg_io(ip, 0x31, 0x1B);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT13);
  w_sh_reg_io(ip, 0x31, 0x1C);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT14);
  w_sh_reg_io(ip, 0x31, 0x1D);
  usleep(100000);

  w_sh_reg_io(ip, 0x30, POT15);
  w_sh_reg_io(ip, 0x31, 0x1E);

  w_sh_reg_io(ip, 0x30, POT16);
  w_sh_reg_io(ip, 0x31, 0x1F);
  usleep(100000);
}

int main(int argc, char** argv)
{

  if (argc < 2) {
    printf("usage: %s <col> \n", argv[0]);
    return 1;
  }
  int col = atoi(argv[1]);
  char ip [256];
  sprintf(ip, "192.168.1.%d", col);
  loshp(ip, col);
  return 0;
}

