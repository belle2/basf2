#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"

#include "daq/slc/apps/dqmviewd/DQMPackageUpdater.h"
#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include "daq/slc/apps/PackageManager.h"

#include <daq/slc/system/Inotify.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Debugger.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>

using namespace Belle2;

void DQMViewMaster::run()
{
  std::vector<DQMFileReader*> reader_v;
  std::map<std::string, int> index_m;
  for (size_t i = 0; i < _manager_v.size(); i++) {
    reader_v.push_back(NULL);
    DQMPackage* monitor = (DQMPackage*)_manager_v[i]->getMonitor();
    index_m.insert(std::map<std::string, int>::value_type(monitor->getFileName(), i));
  }
  Inotify inotify;
  inotify.open();
  inotify.add(_directory, Inotify::FILE_DELETE | Inotify::FILE_MODIFY);
  while (true) {
    InotifyEventList event_v(inotify.wait(5));
    for (size_t i = 0; i < event_v.size(); i++) {
      if (index_m.find(event_v[i].getName()) != _index_m.end() &&
          event_v[i].getMask() == Inotify::FILE_MODIFY) {
        std::string filename = event_v[i].getName();
        int index = _index_m[event_v[i].getName()];
        DQMPackage* monitor = (DQMPackage*)_manager_v[index]->getMonitor();
        if (reader_v[i] == NULL) {
          DQMFileReader* reader = new DQMFileReader();
          if (!reader->init(filename.c_str())) {
            delete reader;
            continue;
          }
          LogFile::debug("Histo entries was found in %s", filename.c_str());
          monitor->setHistMap(reader->getHistMap());
          reader_v[i] = reader;
          _manager_v[i]->init();
          signal(-2);
          sleep(2);
        }
        if (reader_v[i] != NULL) {
          reader_v[i]->update(_manager_v[i]->getPackage());
          if (_manager_v[i]->update()) {
            signal(i);
          }
        }
      }
    }
  }
}
