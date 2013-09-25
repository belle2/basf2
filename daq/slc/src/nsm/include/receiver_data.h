#ifndef _B2DAQ_receiver_data_h
#define _B2DAQ_receiver_data_h

const int receiver_data_revision = 1;

struct receiver_data {
  char script[64];
  uint16 ncopper;
  char  host[1024];
  uint16 port[16];
  uint16 event_size[16];
};

#endif

