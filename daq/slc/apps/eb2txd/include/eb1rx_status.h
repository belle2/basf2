#ifndef _eb1rx_status_h
#define _eb1rx_status_h

namespace Belle2 {

  const int eb1rx_status_revision = 4;

  struct eb_stat {
    uint32 event;
    float byte;
    uint32 connection;
    float nqueue;
    uint32 addr;
    uint32 port;
    uint32 reserved[2];
  };

  struct eb1rx_status {
    eb_stat in[2];
    eb_stat out[2];
  };

}

#endif
