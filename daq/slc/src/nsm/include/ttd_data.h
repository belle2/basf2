#ifndef _B2DAQ_ttd_data_h
#define _B2DAQ_ttd_data_h

const int ttd_data_revision = 1;

struct ttd_data {
  uint16 nftsw;
  uint16 channel[16];
  uint16 trigger_mode[16];
  byte8 used[16];
  char firmware[1024];
};

#endif
