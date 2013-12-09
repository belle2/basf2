#include "daq/slc/apps/dqmviewd/DQMPackageUpdater.h"

#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"

#include "daq/slc/apps/PackageManager.h"

#include <daq/slc/base/Debugger.h>

using namespace Belle2;

void DQMPackageUpdater::start()
{
  _mutex.lock();
  _stopped = false;
  _mutex.unlock();
}

void DQMPackageUpdater::stop()
{
  _mutex.lock();
  _stopped = true;
  _mutex.unlock();
}

bool DQMPackageUpdater::isStopped()
{
  _mutex.lock();
  bool stopped = _stopped;
  _mutex.unlock();
  return stopped;
}

void DQMPackageUpdater::run()
{
  while (true) {
    sleep(5);
    if (!isStopped()) {
      if (_manager->isAvailable()) {
        _manager->update();
        _master->signal(_index);
      }
    }
  }
}
