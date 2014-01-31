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
  std::vector<DQMFileReader>& reader_v(_master->getReaders());
  while (true) {
    _master->lock();
    for (size_t index = 0; index < reader_v.size(); index++) {
      DQMFileReader& reader(reader_v[index]);
      std::string filename = reader.getFileName();
      if (!reader.isReady() && reader.init()) {
        LogFile::debug("Hist entries was found in %s", filename.c_str());
      }
      if (reader.isReady()) {
        //LogFile::debug("Updating %s", reader.getName().c_str());
        reader.update();
      }
    }
    _master->notify();
    _master->unlock();
    sleep(10);
  }
}
