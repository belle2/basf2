#ifndef _Belle2_LogDBManager_h
#define _Belle2_LogDBManager_h

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/Mutex.h>

#include <daq/slc/base/SystemLog.h>

#include <vector>

namespace Belle2 {

  class LogDBManager {

  public:
    LogDBManager(DBInterface* db, const std::string& name)
      : _db(db), _tablename(name) {}
    ~LogDBManager() {}

  public:
    void createTable() throw(DBHandlerException);
    void writeLog(const SystemLog& log) throw(DBHandlerException);
    std::vector<SystemLog> readLogs(const std::string& groupname = "ALL",
                                    const std::string& hostname = "ALL",
                                    const std::string& nodename = "ALL",
                                    int days = -1, int hours = -1, int mins = -1)
    throw(DBHandlerException);

  private:
    DBInterface* _db;
    std::string _tablename;
    Mutex _mutex;

  };

}

#endif

