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
  /*
  _db->connect();
  for (size_t i = 0; i < _data_v.size(); i++) {
    NSMData* data = _data_v[i];
    try {
      _db->execute(Belle2::form("create table \"%s_rev%d\" (%s);",
        data->getName().c_str(),
        data->getRevision(),
        data->toSQLConfig().c_str()));
    } catch (const DBHandlerException& e) {
      LogFile::debug("DB error: %s", e.what());
      sleep(5);
    }
  }
  _db->close();
  */
  while (true) {
    //_db->connect();
    lock();
    for (size_t i = 0; i < _data_v.size(); i++) {
      NSMData* data = _data_v[i];
      if (!data->isAvailable()) {
        try {
          data->open(_comm);
          LogFile::debug("Opend NSM data : %s", data->getName().c_str());
        } catch (const NSMHandlerException& e) {
          LogFile::debug(e.what());
          break;
        }
      }
      /*
      if (data->isAvailable()) {
        _db->execute(Belle2::form("insert into \"%s_rev%d\" values (%s);",
                                   data->getName().c_str(),
                                   data->getRevision(),
                                   data->toSQLValues().c_str()));
      }
      */
    }
    notify();
    unlock();
    //_db->close();
    sleep(5);
  }
}
