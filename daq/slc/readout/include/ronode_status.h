#ifndef ronode_status_h
#define ronode_status_h

const int ronode_status_revision = 1;

struct ronode_status {
  int32 reserved[2];
  uint32 state;
  uint32 expno;
  uint32 runno;
  uint32 subno;
  uint32 ctime;
  uint32 stime;
  struct io_status {
    int32 reserved[3];
    int32 port;
    uint32 count;
    float freq;
    float evtsize;
    float rate;
    uint64 nbyte;
  } io[2];
};

#endif
