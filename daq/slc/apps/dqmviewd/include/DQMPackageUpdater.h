#ifndef _Belle2_DQMPackageUpdaer_hh
#define _Belle2_DQMPackageUpdaer_hh

#include <daq/slc/system/Mutex.h>

namespace Belle2 {

  class PackageManager;
  class DQMViewMaster;

  class DQMPackageUpdater {

  public:
    DQMPackageUpdater(int index, PackageManager* manager,
                      DQMViewMaster* master)
      : _index(index), _manager(manager),
        _master(master), _stopped(true) {}
    ~DQMPackageUpdater() {}

  public:
    void start();
    void stop();
    bool isStopped();
    void run();

  private:
    int _index;
    PackageManager* _manager;
    DQMViewMaster* _master;
    bool _stopped;
    Mutex _mutex;

  };

}

#endif

