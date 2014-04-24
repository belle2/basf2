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
      : _callback(callback), _master(master) {}
    ~DQMPackageUpdater() {}

  public:
    void start();
    void stop();
    bool isStopped();
    void run();

  private:
    DQMViewCallback* _callback;
    DQMViewMaster& _master;

  };

}

#endif

