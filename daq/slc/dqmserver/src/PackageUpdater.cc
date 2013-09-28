#include "PackageUpdater.hh"

#include "PackageManager.hh"
#include "HistoServer.hh"

#include <util/Debugger.hh>

using namespace B2DQM;

void PackageUpdater::run()
{
  while (true) {
    sleep(5);
    if (!isStopped()) {
      if (_manager->isAvailable()) {
        _manager->update();
        _server->signal(_index);
      }
    }
  }
}
