#include "daq/slc/apps/envmonitord/EnvMonitorMaster.h"

#include <daq/slc/database/LoggerObjectTable.h>

#include <daq/slc/system/LogFile.h>

#include <unistd.h>

using namespace Belle2;

void EnvMonitorMaster::run()
{
  while (true) {
    m_db.connect();
    lock();
    for (size_t i = 0; i < m_data_v.size(); i++) {
      NSMData& data(m_data_v[i]);
      if (!data.isAvailable()) {
        try {
          data.open(&m_comm);
          LogFile::debug("Opend NSM data : %s", data.getName().c_str());
        } catch (const NSMHandlerException& e) {
          LogFile::debug(e.what());
          break;
        }
      }
      if (data.isAvailable()) {
        LoggerObjectTable(&m_db).add(data, true);
      }
    }
    notify();
    unlock();
    m_db.close();
    sleep(5);
  }
}
