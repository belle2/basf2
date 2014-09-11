#ifndef _Belle2_PostgreSQLInterface_hh
#define _Belle2_PostgreSQLInterface_hh
///*
#include "daq/slc/database/DBInterface.h"

#include <libpq-fe.h>

namespace Belle2 {

  class PostgreSQLInterface : public DBInterface {

  public:
    PostgreSQLInterface(const std::string& host = "",
                        const std::string& database = "",
                        const std::string& user = "",
                        const std::string& password = "",
                        int port = -1) throw();
    virtual ~PostgreSQLInterface() throw() {}

  public:
    virtual void connect() throw(DBHandlerException);
    virtual bool isConnected() throw();
    virtual void execute_imp(const char* command) throw(DBHandlerException);
    virtual void close() throw(DBHandlerException);
    virtual void clear() throw();
    virtual DBRecordList loadRecords() throw(DBHandlerException);
    virtual bool checkTable(const std::string& tablename) throw(DBHandlerException);
    virtual DBFieldTypeList getTableContents(const std::string& tablename)
    throw(DBHandlerException);

  private:
    PGconn* m_sq_conn;
    PGresult* m_sq_result;

  };

}

//*/
#endif
