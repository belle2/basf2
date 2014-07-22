#ifndef storage_info_all_h
#define storage_info_all_h

const int storage_info_all_revision = 1;

struct storage_info_all {
  struct io_status {
    uint32 count;
    float freq;
    float evtsize;
    float rate;
    uint64 nbyte;
  } io[10];
  uint32 port[2];
  uint32 state;
  uint32 ctime;
  uint32 eflag;
  uint32 expno;
  uint32 runno;
  uint32 subno;
  uint32 rcstate;
  uint32 nnodes;
};

#endif
