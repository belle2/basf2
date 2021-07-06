/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_DBInterface_hh
#define _Belle2_DBInterface_hh

#include "daq/slc/database/DBRecord.h"
#include "daq/slc/system/Mutex.h"

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
                int port);
    virtual ~DBInterface();

  public:
    virtual void connect() = 0;
    virtual bool isConnected() = 0;
    virtual void execute_imp(const char* command) = 0;
    void execute(const char* command, ...);
    void execute(const std::string& command);
    virtual void close() = 0;
    virtual void clear() = 0;
    virtual DBRecordList loadRecords() = 0;
    virtual bool checkTable(const std::string& tablename) = 0;
    virtual DBFieldTypeList getTableContents(const std::string& tablename) = 0;
    void clearRecords() { m_record_v.resize(0); }
    DBRecordList& getRecords() { return m_record_v; }
    const std::string& getHostName() const { return m_host; }
    const std::string& getDatabase() const { return m_database; }
    const std::string& getUserName() const { return m_user; }
    const std::string& getPassword() const { return m_password; }
    int getPort() const { return m_port; }

  protected:
    DBInterface() {}
    void init(const std::string& host,
              const std::string& database,
              const std::string& user,
              const std::string& password,
              int port);

  protected:
    DBRecordList m_record_v;
    std::string m_host;
    std::string m_database;
    std::string m_user;
    std::string m_password;
    int m_port;

  private:
    Mutex m_mutex;
    static const int m_buf_size = 1024 * 1000;
    char* m_buf;

  };

}

#endif
