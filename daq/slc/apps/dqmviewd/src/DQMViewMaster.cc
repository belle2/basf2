#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"

#include "daq/slc/apps/dqmviewd/DQMPackageUpdater.h"

#include "daq/slc/apps/PackageManager.h"

#include <daq/slc/system/Inotify.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/Debugger.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>

using namespace Belle2;

void DQMViewMaster::run()
{
  for (size_t i = 0; i < _manager_v.size(); i++) {
    DQMPackage* monitor = (DQMPackage*)_manager_v[i]->getMonitor();
    _index_m.insert(std::map<std::string, int>::value_type(monitor->getFileName(), i));
    std::string path = _directory + "/" + monitor->getFileName();
    DQMPackageUpdater* updater = new DQMPackageUpdater(i, _manager_v[i], this);
    _updater_v.push_back(updater);
    struct stat st;
    if (::stat(path.c_str(), &st) == 0) {
      while (!_manager_v[i]->init()) {
        sleep(2);
      }
      _manager_v[i]->update();
      updater->start();
      PThread((DQMPackageUpdater*)updater);
    }
  }

  Inotify inotify;
  inotify.open();
  inotify.add(_directory, Inotify::FILE_DELETE |
              Inotify::FILE_CREATE | Inotify::FILE_MODIFY);
  while (true) {
    InotifyEventList event_v(inotify.wait(5));
    for (size_t i = 0; i < event_v.size(); i++) {
      if (_index_m.find(event_v[i].getName()) != _index_m.end()) {
        int index = _index_m[event_v[i].getName()];
        if (event_v[i].getMask() == Inotify::FILE_DELETE) {
          _updater_v[index]->stop();
          _manager_v[index]->clear();
        } else if (event_v[i].getMask() == Inotify::FILE_MODIFY) {
          int count = 0;
          while (!_manager_v[index]->init()) {
            sleep(2);
            count++;
            if (count > 3) break;
          }
          if (count > 3) {
            std::cout << "Failed to initialize with file="
                      << event_v[i].getName() << std::endl;
            continue;
          }
          _manager_v[index]->update();
          _updater_v[index]->start();
        }
        signal(index);
      }
    }
  }
}
