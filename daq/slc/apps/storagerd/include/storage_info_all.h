#ifndef storage_info_all_h
#define storage_info_all_h

const int storage_info_all_revision = 1;

struct storage_info_all {
  uint32 nodeid[20];
  uint32 state[20];
  uint32 expno[20];
  uint32 runno[20];
  uint32 subno[20];
  uint64 stime[20];
  uint64 ctime[20];
  uint32 count[20];
  uint64 nbyte[20];
  double freq[20];
  double evtsize[20];
  double rate[20];
  uint32 connection[2];
  uint32 rcstate;
  uint32 nnodes;
};

#endif
