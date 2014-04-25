#ifndef _Belle2_StoragerMonitor_h
#define _Belle2_StoragerMonitor_h

#include "daq/slc/apps/storagerd/StoragerCallback.h"

namespace Belle2 {

  class StoragerMonitor {

  public:
    StoragerMonitor(StoragerCallback* callback)
      : m_callback(callback) {}
    ~StoragerMonitor() {}

  public:
    void run();
    int checkConnection(const std::string& proc_name, int port);

  private:
    StoragerCallback* m_callback;

  };

}

#endif


