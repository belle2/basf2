#include "DBManager.h"

#include "base/StringUtil.h"
#include "base/Debugger.h"

#include <iostream>

#include <unistd.h>

using namespace Belle2;

Mutex DBManager::__mutex;
DBInterface* DBManager::__db = NULL;

void DBManager::run()
{
  bool db_ready = false;
  while (true) {
    if (!db_ready && _data->isAvailable()) {
      try {
        __mutex.lock();
        __db->execute(Belle2::form("show tables like '%s_rev%d';",
                                   _data->getName().c_str(), _data->getRevision()));
        if (__db->loadRecords().size() == 0) {
          __db->execute(Belle2::form("create table %s_rev%d (%s);",
                                     _data->getName().c_str(), _data->getRevision(),
                                     _data->toSQLConfig().c_str()));
        }
        db_ready = true;
        __mutex.unlock();
      } catch (const DBHandlerException& e) {
        Belle2::debug("DB error: %s", e.what());
        db_ready = false;
        __mutex.unlock();
        sleep(5);
        continue;
      }
    }
    try {
      while (db_ready && _data->isAvailable()) {
        __mutex.lock();
        __db->execute(Belle2::form("insert into %s_rev%d values (%s);",
                                   _data->getName().c_str(),
                                   _data->getRevision(),
                                   _data->toSQLValues().c_str()));
        std::cout << _data->getName() << " : " << _data->toSQLValues() << std::endl;
        __mutex.unlock();
        sleep(5);
      }
    } catch (const DBHandlerException& e) {
      db_ready = false;
      Belle2::debug("DB error: %s", e.what());
      __mutex.unlock();
    }
  }
}
