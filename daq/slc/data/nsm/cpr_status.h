#ifndef _Belle2_cpr_status_h
#define _Belle2_cpr_status_h

extern "C" {
#include <nsm2/nsm2.h>
}

namespace Belle2 {

  const int cpr_status_revision = 1;

  struct cpr_status {
    uint32 state;
    uint32 configid;
  };

}

#endif
