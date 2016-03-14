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
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual bool pause() throw(RCHandlerException);
    virtual bool resume(int subno) throw(RCHandlerException);
    virtual void recover(const DBObject&) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void monitor() throw(RCHandlerException);
    virtual void ok(const char* nodename, const char* data) throw();
    virtual void error(const char* nodename, const char* data) throw();
    void trigft() throw(RCHandlerException);
    void setFTSWID(int id);
    bool setTriggerType(const std::string& type) throw();
    void setTriggerLimit(int limit) throw() { m_trigger_limit = limit; }
    void setDummyRate(int rate) throw() { m_dummy_rate = rate; }

  private:
    RCNode m_ttdnode;
    std::map<std::string, int> m_trgcommands;
    std::string m_trigger_type;
    int m_dummy_rate;
    int m_trigger_limit;
    int m_ftswid;

  private:
    void send(const NSMMessage& msg) throw(RCHandlerException);

  };

}

#endif
