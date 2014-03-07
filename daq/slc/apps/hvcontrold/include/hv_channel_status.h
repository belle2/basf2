#ifndef _Belle2_hv_channel_status_h
#define _Belle2_hv_channel_status_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  const int hv_channel_status_revision = 1;

  struct hv_channel_status {
    uint32 configid;
    uint32 state;
    float  voltage_mon;
    float  current_mon;
  };

}

#endif
