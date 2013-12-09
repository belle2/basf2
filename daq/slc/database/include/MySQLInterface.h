/*
#ifndef _Belle2_MySQLInterface_hh
#define _Belle2_MySQLInterface_hh

#include "daq/slc/database/DBInterface.h"

#include <mysql.h>

namespace Belle2 {

  class MySQLInterface : public DBInterface {

  public:
    MySQLInterface(const std::string& host = "",
       const std::string& database = "",
       const std::string& user = "",
       const std::string& password = "",
       int port = -1) throw();
    virtual ~MySQLInterface() throw() {}

  public:
    virtual void connect() throw(DBHandlerException);
    virtual void execute(const std::string& command) throw(DBHandlerException);
    virtual void close() throw(DBHandlerException);
    virtual void clear() throw();
    virtual DBRecordList& loadRecords() throw(DBHandlerException);

  private:
    MYSQL* _sq_conn;
    MYSQL_RES* _sq_result;

  };

}

#endif
*/
