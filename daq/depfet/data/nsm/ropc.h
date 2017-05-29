#ifndef _Belle2_ropc_h
#define _Belle2_ropc_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  const int ropc_revision = 1;

  struct ropc {
    uint32 nnodes;
    uint32 state;
    uint32 configid;
    uint32 expno;
    uint32 runno;
    uint32 subno;
    uint32 stime;
    uint32 ctime;
    float loadavg;
    float reserved[3];
    struct ronode {
      uint32 nodeid;
      uint32 state;
      uint32 eflag;
      uint32 expno;
      uint32 runno;
      uint32 subno;
      uint32 reserved_i[2];
      uint32 stime;
      uint32 ctime;
      uint32 nevent_in;
      uint32 nqueue_in;
      uint32 connection_in;
      uint32 connection_out;
      uint32 nevent_out;
      uint32 nqueue_out;
      float evtrate_in;
      float evtsize_in;
      float flowrate_in;
      float evtrate_out;
      float evtsize_out;
      float flowrate_out;
      float loadavg;
      float reserved_f[5];
      uint32 evt_nword;
      uint32 evt_format;
      uint32 evt_exp_run;
      uint32 evt_evtno;
      uint32 evt_ctime_trgtype;
      uint32 evt_utime;
      uint32 evt_nodeid;
      uint32 evt_crc_err;
    } node[22];
  };

}

#endif
