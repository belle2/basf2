#ifndef _Belle2_DBManager_h
#define _Belle2_DBManager_h

#include <system/Mutex.h>

#include <nsm/NSMData.h>

#include <database/DBInterface.h>

namespace Belle2 {

  class DBManager {

  private:
    static Mutex __mutex;
    static DBInterface* __db;

  public:
    static void setDB(DBInterface* db) {
      __db = db;
    }

  public:
    DBManager(NSMData* data)
      : _data(data) {}
    ~DBManager() throw() {}

  public:
    void run();

  private:
    NSMData* _data;

  };

}

#endif
