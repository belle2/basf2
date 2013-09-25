#ifndef _B2DAQ_PostgreSQLInterface_hh
#define _B2DAQ_PostgreSQLInterface_hh
/*
#include "DBInterface.hh"

#include <libpq-fe.h>

namespace B2DAQ {

  class PostgreSQLInterface : public DBInterface {

  public:
    PostgreSQLInterface() throw()
      : _sq_conn(NULL), _sq_result(NULL) {}
    virtual ~PostgreSQLInterface() throw() {}

  public:
    virtual void connect(const std::string& host,
			 const std::string& database,
			 const std::string& user,
			 const std::string& password) throw(DBHandlerException);
    virtual void execute(const std::string& command) throw(DBHandlerException);
    virtual void close() throw(DBHandlerException);
    virtual void clear() throw();
    virtual DBRecordList& loadRecords() throw(DBHandlerException);

  private:
    PGconn* _sq_conn;
    PGresult* _sq_result;

  };

}

*/
#endif
