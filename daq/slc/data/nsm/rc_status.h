#ifndef _Belle2_rc_status_h
#define _Belle2_rc_status_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  const int rc_status_revision = 1;

  struct rc_status {
    struct node_status {
      uint32 state;
      uint32 configid;
    } node[20];
    uint32 nnodes;
    uint32 state;
    uint32 configid;
  };

}

#endif
