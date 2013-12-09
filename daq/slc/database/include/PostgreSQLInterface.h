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
    virtual void execute(const std::string& command) throw(DBHandlerException);
    virtual void close() throw(DBHandlerException);
    virtual void clear() throw();
    virtual DBRecordList& loadRecords() throw(DBHandlerException);

  private:
    PGconn* _sq_conn;
    PGresult* _sq_result;

  };

}

//*/
#endif
