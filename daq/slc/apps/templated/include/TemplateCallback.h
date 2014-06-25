#ifndef _Belle2_TemplateCallback_hh
#define _Belle2_TemplateCallback_hh

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/runcontrol/RCCallback.h"

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/PThread.h>

namespace Belle2 {

  class TemplateCallback : public RCCallback {

  public:
    TemplateCallback(const NSMNode& node, const std::string& host, const std::string& port);
    virtual ~TemplateCallback() throw();

  public:
    virtual void init() throw();
    virtual void term() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();

  private:
    ProcessController m_con;
    PThread m_thread;
    NSMData m_data;
    std::string m_host;
    std::string m_port;

  };

}

#endif
