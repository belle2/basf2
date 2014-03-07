#ifndef _Belle2_hv_channel_config_h
#define _Belle2_hv_channel_config_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  struct hv_channel_config {
    uint32 configid;
    uint32 master;
    uint32 crate;
    uint32 slot;
    uint32 channel;
    uint32 turnon;
    float rampup_speed;
    float rampdown_speed;
    float voltage_demand[4];
    float voltage_limit;
    float current_limit;
    float reserved[4];
  };

}

#endif
