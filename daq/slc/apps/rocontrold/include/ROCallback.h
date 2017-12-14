#ifndef _Belle2_ROCallback_h
#define _Belle2_ROCallback_h

#include "daq/slc/apps/rocontrold/EB0Controller.h"
#include "daq/slc/apps/rocontrold/Stream0Controller.h"
#include "daq/slc/apps/rocontrold/Stream1Controller.h"
#include "daq/slc/apps/rocontrold/EB1TXController.h"

#include "daq/slc/system/SharedMemory.h"

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/nsm/NSMData.h>

#include "daq/slc/runcontrol/RCCallback.h"

namespace Belle2 {

  class ROCallback : public RCCallback {

  public:
    ROCallback(const NSMNode& runcontrol);
    virtual ~ROCallback() throw() {}

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual bool pause() throw(RCHandlerException);
    virtual bool resume(int subno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void monitor() throw(RCHandlerException);
    virtual void term() throw();

  public:
    const NSMNode& getRC() const { return m_runcontrol; }
    NSMNode& getRC() { return m_runcontrol; }

  private:
    std::string m_conf;
    NSMNode m_runcontrol;
    EB0Controller m_eb0;
    std::vector<Stream0Controller> m_stream0;
    Stream1Controller m_stream1;
    EB1TXController m_eb1tx;
    std::map<std::string, NSMNode> m_node;
    SharedMemory m_memory;

  };

}

#endif
