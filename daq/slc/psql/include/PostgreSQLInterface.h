/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_PostgreSQLInterface_hh
#define _Belle2_PostgreSQLInterface_hh

#include "daq/slc/database/DBInterface.h"
#include "daq/slc/system/Mutex.h"

typedef struct pg_conn PGconn;
typedef struct pg_result PGresult;

namespace Belle2 {

  class PostgreSQLInterface : public DBInterface {

  public:
    PostgreSQLInterface(const std::string& host,
                        const std::string& database,
                        const std::string& user,
                        const std::string& password,
                        int port);
    PostgreSQLInterface();
    virtual ~PostgreSQLInterface() {}

  public:
    void connect() override;
    bool isConnected() override;
    void execute_imp(const char* command) override;
    void close() override;
    void clear() override;
    DBRecordList loadRecords() override;
    bool checkTable(const std::string& tablename) override;
    DBFieldTypeList getTableContents(const std::string& tablename) override;

  private:
    PGconn* m_sq_conn;
    PGresult* m_sq_result;
    Mutex m_mutex;

  };

}

#endif
