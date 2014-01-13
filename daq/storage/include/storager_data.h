#ifndef storager_data_h
#define storager_data_h

const int storager_data_revision = 1;

struct storager_data {
  unsigned int expno;
  unsigned int runno;
  unsigned int subno;
  unsigned int evtno;
  unsigned int nevts;
  unsigned int nfiles;
  unsigned long long starttime;
  unsigned long long curtime;
  double evtsize;
  double datasize;
  double freq;
  double rate;
};

#endif
