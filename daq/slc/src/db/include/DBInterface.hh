#ifndef _B2DAQ_DBInterface_hh
#define _B2DAQ_DBInterface_hh

#include "DBHandlerException.hh"
#include "DBRecord.hh"

#include <vector>

namespace B2DAQ {

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

  protected:
    DBRecordList _record_v;

  };

}

#endif
