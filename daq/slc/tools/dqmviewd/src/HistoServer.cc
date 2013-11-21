#include "HistoServer.h"

#include "PackageManager.h"
#include "PackageUpdater.h"

#include <dqm/HistoManager.h>

#include <system/Inotify.h>
#include <system/PThread.h>

#include <base/Debugger.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>

using namespace Belle2;

void HistoServer::addManager(const std::string& filename, HistoManager* manager)
{
  _filename_v.push_back(filename);
  _manager_v.push_back(new PackageManager(manager));
}

void HistoServer::signal(int index)
{
  _mutex.lock();
  _cond.broadcast();
  _mutex.unlock();
}

int HistoServer::wait(int sec)
{
  _mutex.lock();
  int index = -1;
  if (_cond.wait(_mutex, sec, 0)) {
    index = _index;
  }
  _mutex.unlock();
  return index;
}

void HistoServer::run()
{
  for (size_t i = 0; i < _manager_v.size(); i++) {
    _index_m.insert(std::map<std::string, int>::value_type(_filename_v[i], i));
    std::string path = _directory + "/" + _filename_v[i];
    _manager_v[i]->setFilePath(path);
    PackageUpdater* updater = new PackageUpdater(i, _manager_v[i], this);
    _updater_v.push_back(updater);
    struct stat st;
    if (::stat(path.c_str(), &st) == 0) {
      while (!_manager_v[i]->init()) {
        sleep(2);
      }
      _manager_v[i]->update();
      updater->start();
      PThread((PackageUpdater*)updater);
    }
  }

  Inotify inotify;
  inotify.open();
  inotify.add(_directory,
              Inotify::FILE_DELETE | Inotify::FILE_CREATE | Inotify::FILE_MODIFY);
  bool created_new = false;
  while (true) {
    InotifyEventList event_v(inotify.wait(5));
    for (size_t i = 0; i < event_v.size(); i++) {
      if (_index_m.find(event_v[i].getName()) != _index_m.end()) {
        int index = _index_m[event_v[i].getName()];
        if (event_v[i].getMask() == Inotify::FILE_DELETE) {
          std::cout << "Detect file delete : " << event_v[i].getName() << std::endl;
          _updater_v[index]->stop();
          _manager_v[index]->clear();
          created_new = false;
        } else if (event_v[i].getMask() == Inotify::FILE_CREATE) {
          created_new = true;
        } else if (event_v[i].getMask() == Inotify::FILE_MODIFY &&
                   created_new == true) {
          std::cout << "Detect file modify : " << event_v[i].getName() << std::endl;
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
          created_new = false;
        }
        signal(index);
      }
    }
  }
}
