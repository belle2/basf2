#ifndef _runcontrol_h
#define _runcontrol_h

const int runcontrol_revision = 1;

struct runcontrol {
  uint32 configid;
  uint32 expno;
  uint32 runno;
  uint32 subno;
  uint32 stime;
  uint32 ctime;
};

#endif
