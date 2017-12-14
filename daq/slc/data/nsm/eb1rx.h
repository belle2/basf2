
const int eb1rx_revision = 4;

struct eb1rx {
  struct eb_stat {
    uint32 event;
    float byte;
    uint32 connection;
    float nqueue;
    uint32 addr;
    uint32 port;
    uint32 reservd[2];
  } in[2];
  struct eb_stat {
    uint32 event;
    float byte;
    uint32 connection;
    float nqueue;
    uint32 addr;
    uint32 port;
    uint32 reservd[2];
  } out[2];
};

