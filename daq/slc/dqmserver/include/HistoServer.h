#ifndef _Belle2_HistoServer_hh
#define _Belle2_HistoServer_hh

#include <system/Mutex.h>
#include <system/Cond.h>

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class HistoManager;
  class PackageManager;
  class PackageUpdater;

  class HistoServer {

  public:
    void setDirectory(const std::string& dir) { _directory = dir; }
    void addManager(const std::string& filename,
                    HistoManager* manager);
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
    std::string _directory;
    std::vector<std::string> _filename_v;
    std::vector<PackageManager*> _manager_v;
    std::vector<PackageUpdater*> _updater_v;
    Belle2::Mutex _mutex;
    Belle2::Cond _cond;
    std::map<std::string, int> _index_m;
    int _index;

  };

}

#endif
