#ifndef _Belle2_PackageUpdater_hh
#define _Belle2_PackageUpdater_hh

#include <system/Mutex.h>

namespace Belle2 {

  class PackageManager;
  class HistoServer;

  class PackageUpdater {

  public:
    PackageUpdater(int index, PackageManager* manager,
                   HistoServer* server)
      : _index(index), _manager(manager),
        _server(server), _stopped(true) {}
    ~PackageUpdater() {}

  public:
    void start() {
      _mutex.lock();
      _stopped = false;
      _mutex.unlock();
    }
    void stop() {
      _mutex.lock();
      _stopped = true;
      _mutex.unlock();
    }
    bool isStopped() {
      _mutex.lock();
      bool stopped = _stopped;
      _mutex.unlock();
      return stopped;
    }
    void run();

  private:
    int _index;
    PackageManager* _manager;
    HistoServer* _server;
    bool _stopped;
    Belle2::Mutex _mutex;

  };

}

#endif

