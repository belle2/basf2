#ifndef storage_info_h
#define storage_info_h

const int storage_info_revision = 1;

struct storage_info {
  unsigned int nodeid;
  unsigned int expno;
  unsigned int runno;
  unsigned int subno;
  unsigned long long stime;
  unsigned int count;
  unsigned long long nbyte;
  unsigned int connection;
};

#endif
