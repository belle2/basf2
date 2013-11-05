#ifndef _Belle2_DBInterface_hh
#define _Belle2_DBInterface_hh

#include "database/DBHandlerException.h"
#include "database/DBRecord.h"

#include <vector>

namespace Belle2 {

  typedef std::vector<DBRecord> DBRecordList;

  class DBInterface {

  public:
    DBInterface() throw() {}
    virtual ~DBInterface() throw();

  public:
    virtual void init() throw(DBHandlerException) = 0;
    virtual void connect(const std::string& host,
                         const std::string& database,
                         const std::string& user,
                         const std::string& password,
                         int port) throw(DBHandlerException) = 0;
    virtual void execute(const std::string& command) throw(DBHandlerException) = 0;
    virtual void close() throw(DBHandlerException) = 0;
    virtual void clear() throw() = 0;
    virtual DBRecordList& loadRecords() throw(DBHandlerException) = 0;
    const std::string& getHostName() const throw() { return _hostname; }
    const std::string& getDatabase() const throw() { return _database; }
    const std::string& getUserName() const throw() { return _username; }
    const std::string& getPassword() const throw() { return _password; }
    int getPort() const throw() { return _port; }

  protected:
    DBRecordList _record_v;
    std::string _hostname;
    std::string _database;
    std::string _username;
    std::string _password;
    int _port;

  };

}

#endif
