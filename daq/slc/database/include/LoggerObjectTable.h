#ifndef _Belle2_LoggerObjectTable_h
#define _Belle2_LoggerObjectTable_h

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/LoggerObject.h>
#include <daq/slc/database/ConfigObject.h>

namespace Belle2 {

  class LoggerObjectTable {

  public:
    LoggerObjectTable(DBInterface* db = NULL) : m_db(db) {}
    ~LoggerObjectTable() throw() {}

  public:
    LoggerObject getLatest(const std::string& nodename,
                           const std::string& tablename_in) throw();
    LoggerObject getList(const std::string& nodename,
                         const std::string& tablename_in) throw();
    int add(const DBObject& obj, bool isroot = true)
    throw(DBHandlerException);

  private:
    DBInterface* m_db;

  };

};

#endif
