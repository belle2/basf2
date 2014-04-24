#ifndef _Belle2_ropc_status_h
#define _Belle2_ropc_status_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  const int ropc_status_revision = 1;

  struct ropc_status {
    struct node_status {
      uint32 state;
      uint32 configid;
    } node[20];
    uint32 state;
    uint32 configid;
  };

}

#endif
