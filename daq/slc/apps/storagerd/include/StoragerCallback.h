#ifndef _Belle2_StoragerCallback_hh
#define _Belle2_StoragerCallback_hh

#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/readout/ProcessController.h>
#include <daq/slc/readout/FlowMonitor.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/PThread.h>

#include <daq/slc/base/ConfigFile.h>

#include <vector>

namespace Belle2 {

  class StoragerCallback : public RCCallback {

  public:
    StoragerCallback();
    virtual ~StoragerCallback() throw();

  public:
    virtual bool initialize(const DBObject& obj) throw();
    virtual bool configure(const DBObject& obj) throw();
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void timeout(NSMCommunicator& com) throw();
    virtual void term() throw();

  private:
    ProcessController m_eb2rx;
    std::vector<ProcessController> m_con;
    std::vector<FlowMonitor> m_flow;
    NSMData m_data;
    SharedEventBuffer m_ibuf;
    SharedEventBuffer m_rbuf;
    ConfigFile m_file;

  };

}

#endif
