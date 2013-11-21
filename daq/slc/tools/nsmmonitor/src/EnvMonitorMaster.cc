#include "EnvMonitorMaster.h"

#include "PackageManager.h"
#include "DBManager.h"

#include <system/PThread.h>

#include <base/StringUtil.h>
#include <base/Debugger.h>
#include <base/ConfigFile.h>

#include <iostream>
#include <cstdlib>

using namespace Belle2;

void EnvMonitorMaster::add(NSMData* data, AbstractMonitor* monitor)
{
  PackageManager* manager = new PackageManager(data, monitor);
  _data_v.push_back(data);
  _manager_v.push_back(manager);
}

void EnvMonitorMaster::signal(int index)
{
  _mutex.lock();
  _index = index;
  _cond.broadcast();
  _mutex.unlock();
}

int EnvMonitorMaster::wait(int sec)
{
  _mutex.lock();
  int index = -1;
  if (_cond.wait(_mutex, sec, 0)) {
    index = _index;
  }
  _mutex.unlock();
  return index;
}

void EnvMonitorMaster::run()
{
  for (size_t i = 0; i < _data_v.size(); i++) {
    _manager_v[i]->init();
  }
  while (true) {
    for (size_t i = 0; i < _data_v.size(); i++) {
      if (!_data_v[i]->isAvailable()) {
        try {
          _data_v[i]->open(_comm);
          PThread(new DBManager(_data_v[i]));
        } catch (const IOException& e) {
          Belle2::debug("NSM data error: %s", e.what());
        }
      }
      _manager_v[i]->update();
    }
    signal(0);
    sleep(5);
  }
}
