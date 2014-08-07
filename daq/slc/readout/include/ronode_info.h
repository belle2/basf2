#ifndef ronode_info_h
#define ronode_info_h

#define IO_NOTUSED 0
#define IO_DISCONNECTED -1

const int ronode_info_revision = 1;

struct ronode_info {
  unsigned int nodeid;
  unsigned int state;
  unsigned int eflag;
  unsigned int expno;
  unsigned int runno;
  unsigned int subno;
  int reserved[4];
  struct io_info {
    int port;
    unsigned int addr;
    unsigned int count;
    unsigned long long nbyte;
  } io[2];
};

#endif
