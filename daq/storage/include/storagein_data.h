#ifndef _storagein_data_h
#define _storagein_data_h

const int storagein_data_revision = 1;

struct storagein_data {
  unsigned int nevts;
  unsigned int nqueue;
  double evtsize;
  double datasize;
  double freq;
  double rate;
};

#endif
