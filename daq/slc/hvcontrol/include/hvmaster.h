#ifndef _hvmaster_h
#define _hvmaster_h

const int hvmaster_revision = 1;

struct hvmaster {
  int32_t rcstate;
  int32_t injection[2];
  int32_t interlock[8];
  int32_t hvstate[8];
};

#endif
