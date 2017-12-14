#ifndef _Belle2_ronode_info_h
#define _Belle2_ronode_info_h

const int ronode_info_revision = 1;

namespace Belle2 {

  struct event_header {
    unsigned int nword;
    unsigned int format;
    unsigned int exp_run;
    unsigned int evtno;
    unsigned int ctime_trgtype;
    unsigned int utime;
    unsigned int nodeid;
    unsigned int crc_err;
  };

  struct ronode_info {
    unsigned int nodeid;
    unsigned int state;
    unsigned int eflag;
    unsigned int expno;
    unsigned int runno;
    unsigned int subno;
    unsigned int reserved[2];
    float reserved_f[5];
    struct io_info {
      int port;
      unsigned int addr;
      unsigned long long nbyte;
      unsigned int count;
    } io[2];
    event_header header;
  };

}

#endif
