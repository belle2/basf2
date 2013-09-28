#ifndef _B2DAQ_ro_data_h
#define _B2DAQ_ro_data_h

const int ro_data_revision = 1;

struct ro_data {
  char script[64];
  uint16 ncopper;
  char  host[1024];
  uint16 port[16];
  uint16 event_size[16];
};

#endif

