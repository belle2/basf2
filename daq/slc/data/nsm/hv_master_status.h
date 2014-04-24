#ifndef _Belle2_hv_master_status_h
#define _Belle2_hv_master_status_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  const int hv_master_status_revision = 1;

  struct hv_master_status {
    byte8  node_state[16];
    uint16 node_configid[16];
    uint32 state;
  };

}

#endif
