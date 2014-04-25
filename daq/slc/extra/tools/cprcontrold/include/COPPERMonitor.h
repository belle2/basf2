#ifndef _Belle2_COPPERMonitor_h
#define _Belle2_COPPERMonitor_h

#include "COPPERCallback.h"

namespace Belle2 {

  class COPPERMonitor {

  public:
    COPPERMonitor(COPPERCallback* callback)
      : m_callback(callback) {}
    ~COPPERMonitor() {}

  public:
    void run();

  private:
    COPPERCallback* m_callback;

  };

}

#endif


