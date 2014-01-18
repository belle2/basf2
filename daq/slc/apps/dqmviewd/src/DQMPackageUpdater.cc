#include "daq/slc/apps/dqmviewd/DQMPackageUpdater.h"

#include "daq/slc/apps/dqmviewd/DQMFileReader.h"
#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"
#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"

#include "daq/slc/apps/PackageManager.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

void DQMPackageUpdater::run()
{
  ConfigFile config("dqm");
  const std::string dumppath = config.get("DQM_DUMP_PATH");
  const std::string mappath = config.get("DQM_MAP_PATH");
  LogFile::debug("DQM_DUMP_PATH=%s", dumppath.c_str());
  LogFile::debug("DQM_MAP_PATH=%s", mappath.c_str());
  unsigned int expno = 0;
  unsigned int runno = 0;
  std::vector<DQMFileReader*> reader_v;
  std::vector<PackageManager*>& manager_v(_master->getManagers());
  for (size_t i = 0; i < manager_v.size(); i++) {
    reader_v.push_back(NULL);
  }
  while (true) {
    //_callback->lock();
    if (expno != _master->getExpNumber() ||
        runno != _master->getRunNumber()) {
      expno = _master->getExpNumber();
      runno = _master->getRunNumber();
      Belle2::debug("creating new DQM records for run # %04d.%06d",
                    (int)expno, (int)runno);
      for (size_t i = 0; i < manager_v.size(); i++) {
        if (reader_v[i] != NULL) {
          reader_v[i]->dump(dumppath, expno, runno);
        }
      }
    }
    //_callback->unlock();
    for (size_t index = 0; index < manager_v.size(); index++) {
      DQMPackage* monitor = (DQMPackage*)manager_v[index]->getMonitor();
      std::string filename = monitor->getFileName();
      if (reader_v[index] == NULL) {
        DQMFileReader* reader = new DQMFileReader(monitor->getPackage()->getName());
        if (!reader->init((mappath + "/" + filename).c_str())) {
          delete reader;
          continue;
        }
        LogFile::debug("Hist entries was found in %s", filename.c_str());
        monitor->setHistMap(reader->getHistMap());
        reader_v[index] = reader;
        manager_v[index]->init();
        _master->signal(-2);
      } else if (reader_v[index] != NULL) {
        reader_v[index]->update(manager_v[index]->getPackage());
        if (manager_v[index]->update()) {
          _master->signal(index);
        }
      }
    }
    sleep(5);
  }
}
