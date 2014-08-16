#ifndef ronode_status_h
#define ronode_status_h

const int ronode_status_revision = 3;

struct ronode_status {
  uint32 reserved[3];
  uint32 nodeid;
  uint32 state;
  uint32 ctime;
  uint32 eflag;
  uint32 expno;
  uint32 runno;
  uint32 subno;
  uint32 reserved2[2];
  struct io_status {
    uint32 nqueue;
    uint32 state;
    uint32 count;
    uint32 reserved_i;
    float freq;
    float evtsize;
    float rate;
    float reserved_f;
  } io[2];
};

#endif
