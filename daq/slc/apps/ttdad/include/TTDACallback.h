#ifndef _Belle2_TTDACallback_hh
#define _Belle2_TTDACallback_hh

#include "daq/slc/runcontrol/RCCallback.h"
#include "daq/slc/runcontrol/RCNode.h"

#include <daq/slc/nsm/NSMData.h>

#include <map>

namespace Belle2 {

  class TTDACallback : public RCCallback {

  public:
    TTDACallback(const RCNode& ttd);
    virtual ~TTDACallback() throw() {}

  public:
    virtual bool initialize(const DBObject& obj) throw();
    virtual bool configure(const DBObject& obj) throw();
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void pause() throw(RCHandlerException);
    virtual void resume() throw(RCHandlerException);
    virtual void recover(const DBObject&) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void timeout(NSMCommunicator& com) throw();
    void trigft() throw(RCHandlerException);
    bool setTriggerType(const std::string& type) throw();

  private:
    RCNode m_ttdnode;
    std::map<std::string, int> m_trgcommands;
    std::string m_trigger_type;

  private:
    void send(const NSMMessage& msg) throw(RCHandlerException);

  };

}

#endif
