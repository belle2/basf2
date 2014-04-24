#ifndef _Belle2_DBInterface_hh
#define _Belle2_DBInterface_hh

#include "daq/slc/database/DBHandlerException.h"
#include "daq/slc/database/DBRecord.h"

#include <vector>
#include <map>

namespace Belle2 {

  typedef std::vector<DBRecord> DBRecordList;
  typedef std::map<std::string, std::string> DBFieldTypeList;

  class DBInterface {

  public:
    DBInterface(const std::string& host,
                const std::string& database,
                const std::string& user,
                const std::string& password,
                int port) throw();
    virtual ~DBInterface() throw();

  public:
    virtual void connect() throw(DBHandlerException) = 0;
    virtual void execute_imp(const std::string& command) throw(DBHandlerException) = 0;
    void execute(const std::string& command, ...) throw(DBHandlerException);
    virtual void close() throw(DBHandlerException) = 0;
    virtual void clear() throw() = 0;
    virtual DBRecordList loadRecords() throw(DBHandlerException) = 0;
    virtual bool checkTable(const std::string& tablename) throw(DBHandlerException) = 0;
    virtual DBFieldTypeList getTableContents(const std::string& tablename)
    throw(DBHandlerException) = 0;
    void clearRecords() throw(DBHandlerException) { _record_v.resize(0); }
    DBRecordList& getRecords() throw(DBHandlerException) { return _record_v; }
    const std::string& getHostName() const throw() { return _host; }
    const std::string& getDatabase() const throw() { return _database; }
    const std::string& getUserName() const throw() { return _user; }
    const std::string& getPassword() const throw() { return _password; }
    int getPort() const throw() { return _port; }

  protected:
    DBRecordList _record_v;
    std::string _host;
    std::string _database;
    std::string _user;
    std::string _password;
    int _port;

  };

}

#endif
