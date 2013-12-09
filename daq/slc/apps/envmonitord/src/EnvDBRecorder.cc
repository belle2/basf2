#include "daq/slc/apps/envmonitord/EnvDBRecorder.h"

#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/Debugger.h"

#include <unistd.h>

using namespace Belle2;

Mutex EnvDBRecorder::__mutex;
DBInterface* EnvDBRecorder::__db = NULL;

void EnvDBRecorder::run()
{
  bool db_ready = false;
  while (true) {
    if (!db_ready) {
      try {
        __mutex.lock();
        for (NSMDataMap::iterator it = _monitor->getDataMap().begin();
             it != _monitor->getDataMap().end(); it++) {
          NSMData* data = it->second;
          __db->connect();
          __db->execute(Belle2::form("create table \"%s_rev%d\" (%s);",
                                     data->getName().c_str(),
                                     data->getRevision(),
                                     data->toSQLConfig().c_str()));
          __db->close();
        }
        db_ready = true;
        __mutex.unlock();
      } catch (const DBHandlerException& e) {
        Belle2::debug("DB error: %s", e.what());
        db_ready = false;
        __db->close();
        __mutex.unlock();
        sleep(5);
        continue;
      }
    }
    try {
      while (db_ready) {
        __mutex.lock();
        for (NSMDataMap::iterator it = _monitor->getDataMap().begin();
             it != _monitor->getDataMap().end(); it++) {
          NSMData* data = it->second;
          __db->connect();
          __db->execute(Belle2::form("insert into \"%s_rev%d\" values (%s);",
                                     data->getName().c_str(),
                                     data->getRevision(),
                                     data->toSQLValues().c_str()));
          __db->close();
          __mutex.unlock();
          sleep(5);
        }
      }
    } catch (const DBHandlerException& e) {
      db_ready = false;
      Belle2::debug("DB error: %s", e.what());
      __db->close();
      __mutex.unlock();
    }
  }
}
