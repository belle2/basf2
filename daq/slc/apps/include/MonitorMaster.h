#ifndef _Belle2_MonitorMaster_hh
#define _Belle2_MonitorMaster_hh

#include <daq/slc/apps/PackageManager.h>

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>

#include <string>
#include <vector>
#include <map>

namespace Belle2 {


  class MonitorMaster {

  public:
    void addManager(PackageManager* manager);
    std::vector<PackageManager*>& getManagers() { return _manager_v; }
    PackageManager* getManager(int i) { return _manager_v[i]; }
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }

  public:
    void signal(int index);
    int wait(int sec);

  protected:
    std::vector<PackageManager*> _manager_v;
    Belle2::Mutex _mutex;
    Belle2::Cond _cond;
    std::map<std::string, int> _index_m;
    int _index;

  };

}

#endif
