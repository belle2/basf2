#ifndef _Belle2_EnvMonitorMaster_hh
#define _Belle2_EnvMonitorMaster_hh

#include "dqm/AbstractMonitor.h"

#include <system/Mutex.h>
#include <system/Cond.h>

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class PackageManager;

  class EnvMonitorMaster {

  public:
    EnvMonitorMaster(NSMCommunicator* comm)
      : _comm(comm) {}
    ~EnvMonitorMaster() throw() {}

  public:
    void add(NSMData* data, AbstractMonitor* monitor);
    std::vector<PackageManager*>& getManagers() { return _manager_v; }
    PackageManager* getManager(int i) { return _manager_v[i]; }
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }

  public:
    void signal(int index);
    int wait(int sec);

  public:
    void run();

  private:
    NSMCommunicator* _comm;
    std::vector<PackageManager*> _manager_v;
    std::vector<NSMData*> _data_v;
    Mutex _mutex;
    Cond _cond;
    std::map<std::string, int> _index_m;
    int _index;

  };

}

#endif
