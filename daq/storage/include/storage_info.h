#ifndef storage_info_h
#define storage_info_h

const int storage_info_revision = 1;

struct storage_info {
  unsigned int nodeid;
  unsigned int expno;
  unsigned int runno;
  unsigned int subno;
  unsigned int evtno;
  unsigned long long stime;
  unsigned long long ctime;
  unsigned int count_in;
  unsigned int count_out;
  unsigned long long nword_in;
  unsigned long long nword_out;
};

#endif
