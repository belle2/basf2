#ifndef _Belle2_StoragerCallback_hh
#define _Belle2_StoragerCallback_hh

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/nsm/RCCallback.h"
#include "daq/slc/nsm/NSMData.h"

#include <vector>

namespace Belle2 {

  class StoragerCallback : public RCCallback {

  public:
    StoragerCallback(NSMNode* node);
    virtual ~StoragerCallback() throw();

  public:
    size_t getNControllers() const { return _con.size(); }
    ProcessController& getController(int n) { return _con[n]; }
    std::vector<ProcessController>& getControllers() { return _con; }
    NSMData* getData() { return _data; }

  public:
    virtual void init() throw();
    virtual void term() throw();
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();

  private:
    std::vector<ProcessController> _con;
    NSMData* _data;

  };

}

#endif
