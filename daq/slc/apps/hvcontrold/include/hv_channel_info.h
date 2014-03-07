#ifndef _Belle2_hv_channel_info_h
#define _Belle2_hv_channel_info_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  struct hv_channel_info {
    uint32 master;
    uint32 crate;
    uint32 slot;
    uint32 channel;
    uint32 turnon;
    float rampup_speed;
    float rampdown_speed;
    float voltage_limit;
    float current_limit;
    float voltage_set[4];
    float reserved[4];
  };

}

#endif
