#ifndef _ca2nsm_h
#define _ca2nsm_h

//#include <nsm2/nsm2.h>

#define MAX_PV 100
#define MAX_NAME_LEN 40

const int ca2nsm_revision = 1;

struct ca2nsm {
  struct pv_info {
    int32 chid;
    uint32 timestamp;
    char name[40];
    char data[40];
  } pv[100];
};

#endif
