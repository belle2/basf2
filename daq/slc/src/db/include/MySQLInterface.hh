#ifndef _B2DAQ_MySQLInterface_hh
#define _B2DAQ_MySQLInterface_hh

#include "DBInterface.hh"

#include <mysql.h>

namespace B2DAQ {

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
