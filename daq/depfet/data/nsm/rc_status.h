#ifndef _Belle2_rc_status_h
#define _Belle2_rc_status_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  const int rc_status_revision = 4;

  struct rc_status {
    uint32 nnodes;
    uint32 state;
    uint32 configid;
    uint32 expno;
    uint32 runno;
    uint32 subno;
    uint32 stime;
    uint32 ctime;
    struct node_status {
      uint32 state;
      uint32 configid;
      uint32 excluded;
      uint32 eflag;
    } node[20];
  };

}

#endif
