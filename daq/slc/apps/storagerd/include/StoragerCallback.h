#ifndef _Belle2_StoragerCallback_hh
#define _Belle2_StoragerCallback_hh

#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/readout/ProcessController.h>
#include <daq/slc/readout/FlowMonitor.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/PThread.h>

#include <vector>

namespace Belle2 {

  class StoragerCallback : public RCCallback {

  public:
    StoragerCallback(const NSMNode& node);
    virtual ~StoragerCallback() throw();

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
    std::vector<FlowMonitor> m_flow;
    NSMData m_data;
    SharedEventBuffer m_ibuf;
    SharedEventBuffer m_rbuf;

  };

}

#endif
