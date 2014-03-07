#ifndef _Belle2_hv_status_h
#define _Belle2_hv_status_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  const int hv_status_revision = 1;

  struct hv_status {
    byte8  ch_state[20];
    float voltage_mon[20];
    float current_mon[20];
    uint32 state;
  };

}

#endif
