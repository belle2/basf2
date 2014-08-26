#ifndef _Belle2_ronode_info_h
#define _Belle2_ronode_info_h

extern "C" {
#include <nsm2/nsm2.h>
}

const int ronode_info_revision = 1;

struct ronode_info {
  unsigned int nodeid;
  unsigned int state;
  unsigned int eflag;
  unsigned int expno;
  unsigned int runno;
  unsigned int subno;
  unsigned int reserved[2];
  float reserved_f[4];
  struct io_info {
    int port;
    unsigned int addr;
    unsigned int count;
    unsigned long long nbyte;
  } io[2];
};

#endif
