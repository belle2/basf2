#ifndef _Belle2_TemplateCallback_hh
#define _Belle2_TemplateCallback_hh

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/runcontrol/RCCallback.h"

namespace Belle2 {

  class TemplateCallback : public RCCallback {

  public:
    TemplateCallback(const NSMNode& node);
    virtual ~TemplateCallback() throw();

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
    ProcessController _con;
    std::string _path;

  };

}

#endif
