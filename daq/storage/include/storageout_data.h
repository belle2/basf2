#ifndef _storageout_data_h
#define _storageout_data_h

const int storageout_data_revision = 1;

struct storageout_data {
  unsigned int nevts;
  unsigned int nqueue;
  double evtsize;
  double datasize;
  double freq;
  double rate;
};

#endif
