#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"

#include "daq/slc/apps/dqmviewd/DQMPackageUpdater.h"
#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

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
  std::vector<DQMFileReader*> reader_v;
  for (size_t i = 0; i < _manager_v.size(); i++) {
    DQMPackageUpdater* updater = new DQMPackageUpdater(i, _manager_v[i], this);
    _updater_v.push_back(updater);
    PThread((DQMPackageUpdater*)updater);
    reader_v.push_back(NULL);
  }
  while (true) {
    for (size_t i = 0; i < _manager_v.size(); i++) {
      sleep(5);
      DQMPackage* monitor = (DQMPackage*)_manager_v[i]->getMonitor();
      std::string filename = _directory + "/" + monitor->getFileName();
      struct stat st;
      if (::stat(filename.c_str(), &st) == 0) {
        if (!_manager_v[i]->isAvailable()) {
          DQMFileReader* reader = new DQMFileReader();
          if (!reader->init(filename.c_str(), _manager_v[i]->getPackage())) {
            delete reader;
            Belle2::debug("No entry was found in %s", filename.c_str());
            continue;
          }
          Belle2::debug("Histo entries was found in %s", filename.c_str());
          reader_v[i] = reader;
          _manager_v[i]->init();
          _updater_v[i]->start();
        }
        if (reader_v[i] != NULL) {
          reader_v[i]->update(_manager_v[i]->getPackage());
          signal(i);
        }
      }
    }
  }
}
