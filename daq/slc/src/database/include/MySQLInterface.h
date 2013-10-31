#ifndef _Belle2_MySQLInterface_hh
#define _Belle2_MySQLInterface_hh

#include "DBInterface.h"

#include <mysql.h>

namespace Belle2 {

  class MySQLInterface : public DBInterface {

  public:
    MySQLInterface() throw()
      : _sq_conn(NULL), _sq_result(NULL) {}
    virtual ~MySQLInterface() throw() {}

  public:
    virtual void init() throw(DBHandlerException);
    virtual void connect(const std::string& host,
                         const std::string& database,
                         const std::string& user,
                         const std::string& password,
                         int port) throw(DBHandlerException);
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
