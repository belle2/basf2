#ifndef _Belle2_DBRunInfoHandler_hh
#define _Belle2_DBRunInfoHandler_hh

#include <database/DBInterface.h>

#include <nsm/RunStatus.h>
#include <nsm/RunConfig.h>

#include <base/NodeSystem.h>

namespace Belle2 {

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

