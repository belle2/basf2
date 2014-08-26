#ifndef _Belle2_ronode_status_h
#define _Belle2_ronode_status_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  const int ronode_status_revision = 4;

  struct ronode_status {
    uint32 nodeid;
    uint32 stime;
    uint32 ctime;
    uint32 state;
    uint32 eflag;
    uint32 expno;
    uint32 runno;
    uint32 subno;
    uint32 nevent_in;
    uint32 nqueue_in;
    uint32 connection_in;
    uint32 connection_out;
    uint32 nevent_out;
    uint32 nqueue_out;
    int32 reserved_i[2];
    float evtrate_in;
    float evtsize_in;
    float flowrate_in;
    float evtrate_out;
    float evtsize_out;
    float flowrate_out;
    float loadave;
    float reserved_f[5];
  };

}

#endif
