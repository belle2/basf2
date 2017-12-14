#ifndef _Belle2_rfunitinfo_h
#define _Belle2_rfunitinfo_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  const int rfunitinfo_revision = 3;

  struct rfunitinfo {
    struct rfnodeinfo {
      int32 state;
      int32 nevent_in;
      int32 nqueue_in;
      int32 nevent_out;
      int32 nqueue_out;
      int32 eflag;
      int32 i_reserved[9];
      float flowrate_in;
      float flowrate_out;
      float evtsize_in;
      float evtsize_out;
      float evtrate_in;
      float evtrate_out;
      float loadavg;
      float r_reserved[9];
    } nodeinfo[24];
    uint32 nnodes;
    uint32 ctime;
    uint32 rcstate;
    uint32 state;
  };

}

#endif
