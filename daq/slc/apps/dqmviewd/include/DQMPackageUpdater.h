#ifndef _Belle2_DQMPackageUpdaer_hh
#define _Belle2_DQMPackageUpdaer_hh

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>

namespace Belle2 {

  class DQMViewCallback;
  class DQMViewMaster;

  class DQMPackageUpdater {

  public:
    DQMPackageUpdater(DQMViewCallback* callback,
                      DQMViewMaster& master)
      : m_callback(callback), m_master(master) {}
    ~DQMPackageUpdater() {}

  public:
    void start();
    void stop();
    bool isStopped();
    void run();

  private:
    DQMViewCallback* m_callback;
    DQMViewMaster& m_master;

  };

}

#endif

