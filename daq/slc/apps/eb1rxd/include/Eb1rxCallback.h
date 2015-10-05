#ifndef _Belle2_Eb1rxCallback_h
#define _Belle2_Eb1rxCallback_h

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/readout/ProcessController.h>
#include <daq/slc/readout/FlowMonitor.h>

namespace Belle2 {

  class Eb1rxCallback : public RCCallback {

  public:
    Eb1rxCallback();
    virtual ~Eb1rxCallback() throw();

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual bool pause() throw(RCHandlerException);
    virtual bool resume(int subno) throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void monitor() throw(RCHandlerException);

  private:
    ProcessController m_con;
    FlowMonitor m_flow;
    std::string m_executable;

  };

}

#endif
