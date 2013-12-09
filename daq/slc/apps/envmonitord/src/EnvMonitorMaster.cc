#include "daq/slc/apps/envmonitord/EnvMonitorMaster.h"

#include "daq/slc/apps/envmonitord/EnvDBRecorder.h"

#include "daq/slc/apps/PackageManager.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>

using namespace Belle2;

void EnvMonitorMaster::run()
{
  while (true) {
    for (size_t i = 0; i < _manager_v.size(); i++) {
      if (!_manager_v[i]->isAvailable()) {
        EnvMonitorPackage* monitor = (EnvMonitorPackage*)_manager_v[i]->getMonitor();
        for (NSMDataMap::iterator it = monitor->getDataMap().begin();
             it != monitor->getDataMap().end(); it++) {
          NSMData* data = it->second;
          if (!data->isAvailable()) {
            try {
              data->open(_comm);
            } catch (const NSMHandlerException& e) {
              break;
            }
          }
        }
        _manager_v[i]->init();
        PThread(new EnvDBRecorder(monitor));
      }
    }
    for (size_t i = 0; i < _manager_v.size(); i++) {
      if (_manager_v[i]->isAvailable()) {
        _manager_v[i]->update();
      }
    }
    signal(0);
    sleep(5);
  }
}
