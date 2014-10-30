#ifndef _Belle2_ROCallback_hh
#define _Belle2_ROCallback_hh

#include "daq/slc/readout/ProcessController.h"

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/nsm/NSMData.h>

#include "daq/slc/runcontrol/RCCallback.h"
#include "daq/slc/readout/FlowMonitor.h"

namespace Belle2 {

  class ROCallback : public RCCallback {

  public:
    ROCallback(const NSMNode& node, const std::string& conf);
    virtual ~ROCallback() throw();

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
    virtual void timeout() throw();

  private:
    std::vector<ProcessController> m_con;
    FlowMonitor m_flow;
    NSMData m_data;
    ConfigFile m_file;

  };

}

#endif
