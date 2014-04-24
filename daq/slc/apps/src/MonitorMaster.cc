#include "daq/slc/apps/MonitorMaster.h"

#include "daq/slc/apps/PackageManager.h"

using namespace Belle2;

void MonitorMaster::addManager(PackageManager* manager)
{
  _manager_v.push_back(manager);
}

void MonitorMaster::signal(int index)
{
  _mutex.lock();
  _index = index + 1;
  _cond.broadcast();
  _mutex.unlock();
}

int MonitorMaster::wait(int sec)
{
  _mutex.lock();
  int index = 0;
  if (_cond.wait(_mutex, sec, 0)) {
    index = _index;
  }
  _mutex.unlock();
  return index;
}

