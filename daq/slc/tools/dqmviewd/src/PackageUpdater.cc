#include "PackageUpdater.h"

#include "PackageManager.h"
#include "HistoServer.h"

#include <base/Debugger.h>

using namespace Belle2;

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
