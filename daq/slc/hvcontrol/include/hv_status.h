#ifndef _Belle2_hv_status_h
#define _Belle2_hv_status_h

extern "C" {
#include <nsm2/nsm2.h>
}

#define MAX_HVCHANNELS 200

namespace Belle2 {

  const int hv_status_revision = 1;

  struct hv_status {
    struct channel_status {
      uint32 state;
      float voltage_mon;
      float current_mon;
    } channel[MAX_HVCHANNELS];
    uint32 state;
    uint32 configid;
  };

}

#endif
