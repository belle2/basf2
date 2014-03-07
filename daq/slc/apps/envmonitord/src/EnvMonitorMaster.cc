#include "daq/slc/apps/envmonitord/EnvMonitorMaster.h"

#include "daq/slc/apps/PackageManager.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>

using namespace Belle2;

void EnvMonitorMaster::run()
{
  while (true) {
    if (_db != NULL) _db->connect();
    lock();
    for (size_t i = 0; i < _data_v.size(); i++) {
      NSMData* data = _data_v[i];
      if (!data->isAvailable()) {
        try {
          data->open(_comm);
          data->parse();
          LogFile::debug("Opend NSM data : %s", data->getName().c_str());
          try {
            data->openDB(_db);
          } catch (const DBHandlerException& e) {
            LogFile::debug("DB error: %s", e.what());
          }
        } catch (const NSMHandlerException& e) {
          LogFile::debug(e.what());
          break;
        }
      }
      if (data->isAvailable()) {
        data->writeDB(_db);
      }
    }
    notify();
    unlock();
    if (_db != NULL) _db->close();
    sleep(5);
  }
}
