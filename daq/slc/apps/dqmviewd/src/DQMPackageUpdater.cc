#include "daq/slc/apps/dqmviewd/DQMPackageUpdater.h"

#include "daq/slc/apps/dqmviewd/DQMViewMaster.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

void DQMPackageUpdater::run()
{
  std::vector<DQMFileReader>& reader_v(m_master.getReaders());
  while (true) {
    m_master.lock();
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
    m_master.notify();
    m_master.unlock();
    sleep(10);
  }
}
