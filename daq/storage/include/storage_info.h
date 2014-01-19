#ifndef storage_info_h
#define storage_info_h

const int storage_info_revision = 1;

struct storage_info {
  unsigned int expno;
  unsigned int runno;
  unsigned int subno;
  unsigned int evtno;
  unsigned int nevts;
  unsigned int nfiles;
  unsigned long long start_time_tag;
  unsigned long long time_tag;
  unsigned long long nword_in;
  unsigned long long nword_out;
};

#endif
