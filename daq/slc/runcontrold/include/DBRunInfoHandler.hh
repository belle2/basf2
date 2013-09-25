#ifndef _B2DAQ_DBRunInfoHandler_hh
#define _B2DAQ_DBRunInfoHandler_hh

#include <db/DBInterface.hh>

#include <nsm/RunStatus.hh>
#include <nsm/RunConfig.hh>

#include <node/NodeSystem.hh>

namespace B2DAQ {

  class DBRunInfoHandler {

  public:
    DBRunInfoHandler(DBInterface* db, RunStatus* status, RunConfig* config) throw()
      : _db(db), _status(status), _config(config) {}
    virtual ~DBRunInfoHandler() throw() {}

  public:
    DBInterface* getDB() { return _db; }
    void createRunConfigTable() throw(DBHandlerException);
    void createRunStatusTable() throw(DBHandlerException);
    void writeRunConfigTable() throw(DBHandlerException);
    void writeRunStatusTable() throw(DBHandlerException);

  protected:
    DBInterface* _db;
    RunStatus* _status;
    RunConfig* _config;

  };

}

#endif

