#ifndef _Belle2_storage_h
#define _Belle2_storage_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  const int storage_revision = 9;

  struct storage {
    uint32 stime;
    uint32 ctime;
    uint32 state;
    uint32 eflag;
    uint32 expno;
    uint32 runno;
    uint32 subno;
    uint32 nfiles;
    uint32 ndisks;
    uint32 diskid;
    uint32 nnodes;
    float nbytes;
    float loadavg;
    struct disk {
      int32 nfiles;
      float available;
      float size;
    } disk[12];
    struct node {
      uint32 connection_in;
      uint32 connection_out;
      uint32 eflag;
      uint32 ctime;
      uint32 nevent_in;
      uint32 nqueue_in;
      uint32 nevent_out;
      uint32 nqueue_out;
      int32 reserved_i[2];
      float evtrate_in;
      float evtsize_in;
      float flowrate_in;
      float evtrate_out;
      float evtsize_out;
      float flowrate_out;
      float reserved_f[2];
    } node[8];
    struct eb2in {
      uint32 event;
      float byte;
      uint32 connection;
      float nqueue;
    } eb2in[2];
    struct eb2out {
      uint32 event;
      float byte;
      uint32 connection;
      float nqueue;
    } eb2out[1];
  };

}
#endif
