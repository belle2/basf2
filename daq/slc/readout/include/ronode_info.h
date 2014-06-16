#ifndef ronode_info_h
#define ronode_info_h

#define IO_NOTUSED 0
#define IO_DISCONNECTED -1

const int ronode_info_revision = 1;

struct ronode_info {
  int reserved[2];
  unsigned int expno;
  unsigned int runno;
  unsigned int subno;
  unsigned int stime;
  struct io_info {
    int reserved[2];
    int port;
    unsigned int count;
    long long nbyte;
  } io[2];
};

#endif
