#ifndef _Belle2_rorc_status_h
#define _Belle2_rorc_status_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  const int rorc_status_revision = 1;

  struct rorc_status {
    uint32 nnodes;
    uint32 state;
    uint32 configid;
    uint32 expno;
    uint32 runno;
    uint32 subno;
    uint32 stime;
    uint32 ctime;
    struct ronode_status {
      uint32 state;
      uint32 configid;
      uint32 excluded;
      uint32 eflag;
    } node[20];
    struct ro_status {
      uint32 connection_in;
      uint32 nevent_in;
      uint32 nqueue_in;
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
    } ro[20];
  };

}

#endif
