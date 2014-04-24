#ifndef _mgt_h
#define _mgt_h

#if defined (__cplusplus)
extern "C" {
#endif

#define MGT_RDONLY  0
#define MGT_RDWR    1
#define MGT_MMAP    2

#define MGT_CCLK  0x74
#define MGT_CONF  0x75
#define MGT_CVER  0x76
#define MGT_TYP1  0x7d
#define MGT_TYP2  0x7e

#define M012_SERIAL      7
#define M012_SELECTMAP   6

typedef struct Mgt {
  int fd;
  volatile unsigned long* xmemp;
  int slot;
  int delay;
  int window;
  int adc_th;
  int tdc_th;
  int ped[48];
} Mgt_t;

int mgt_check_error(Mgt_t* mgt);
int mgt_check_FEE(Mgt_t* mgt);
int mgt_execute(Mgt_t* mgt, int ctl_code);
int mgt_set_param(Mgt_t* mgt, int address, int value);
int mgt_set_param2(Mgt_t* mgt, int address, int value);
int mgt_get_param(Mgt_t* mgt, int address);

Mgt_t* mgt_open(int slot, int mode);
void mgt_close(Mgt_t* mgt);
int mgt_read(Mgt_t* mgt, int reg);
int mgt_write(Mgt_t* mgt, int reg, int val);
void mgt_write_fpga(Mgt_t* mgt, int m012, int ch, int n);
int mgt_boot_fpga(Mgt_t* mgt, const char* file,
                  int verbose, int forced, int m012);
void mgt_dump_fpga(int conf, const char* str);

Mgt_t* mgt_boot(int slot, const char* path, int* board_type,
                int* firmware, int* hardware);

#if defined (__cplusplus)
}
#endif

#endif
