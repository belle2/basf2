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
    virtual bool isConnected() throw() = 0;
    virtual void execute_imp(const char* command) throw(DBHandlerException) = 0;
    void execute(const char* command, ...) throw(DBHandlerException);
    virtual void close() throw(DBHandlerException) = 0;
    virtual void clear() throw() = 0;
    virtual DBRecordList loadRecords() throw(DBHandlerException) = 0;
    virtual bool checkTable(const std::string& tablename) throw(DBHandlerException) = 0;
    virtual DBFieldTypeList getTableContents(const std::string& tablename)
    throw(DBHandlerException) = 0;
    void clearRecords() throw(DBHandlerException) { m_record_v.resize(0); }
    DBRecordList& getRecords() throw(DBHandlerException) { return m_record_v; }
    const std::string& getHostName() const throw() { return m_host; }
    const std::string& getDatabase() const throw() { return m_database; }
    const std::string& getUserName() const throw() { return m_user; }
    const std::string& getPassword() const throw() { return m_password; }
    int getPort() const throw() { return m_port; }

  protected:
    DBRecordList m_record_v;
    std::string m_host;
    std::string m_database;
    std::string m_user;
    std::string m_password;
    int m_port;
    char m_buf[1024 * 100];

  };

}

#endif
