#ifndef storage_status_h
#define storage_status_h

const int storage_status_revision = 1;

struct storage_status {
  uint32 reserved[2];
  uint32 state;
  uint32 ctime;
  uint32 eflag;
  uint32 expno;
  uint32 runno;
  uint32 subno;
  uint32 nfiles;
  float nbytes;
  float diskusage[12];
  float disksize[12];
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
