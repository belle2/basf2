#ifndef storage_info_all_h
#define storage_info_all_h

const int storage_info_all_revision = 1;

struct storage_info_all {
  uint32 reserved[2];
  uint32 state;
  uint32 ctime;
  uint32 eflag;
  uint32 expno;
  uint32 runno;
  uint32 subno;
  struct io_status {
    uint32 nqueue;
    uint32 state;
    uint32 count;
    float freq;
    float evtsize;
    float rate;
  } io[14];
};

#endif
