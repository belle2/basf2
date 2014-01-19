#ifndef _Belle2_COPPERMonitor_h
#define _Belle2_COPPERMonitor_h

#include "COPPERCallback.h"

namespace Belle2 {

  class COPPERMonitor {

  public:
    COPPERMonitor(COPPERCallback* callback)
      : _callback(callback) {}
    ~COPPERMonitor() {}

  public:
    void run();

  private:
    COPPERCallback* _callback;

  };

}

#endif


