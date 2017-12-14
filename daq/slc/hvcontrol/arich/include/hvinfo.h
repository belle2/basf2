#ifndef _hvinfo_h
#define _hvinfo_h

struct hvinfo {
  unsigned int record_time;
  int crate;
  int slot;
  int channel;
  int switchon;
  float vdemand;
  float vmon;
  float cmon;
};

#endif
