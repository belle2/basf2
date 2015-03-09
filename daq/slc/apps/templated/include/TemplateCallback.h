#ifndef _Belle2_TemplateCallback_h
#define _Belle2_TemplateCallback_h

#include "daq/slc/runcontrol/RCCallback.h"

namespace Belle2 {

  class TemplateCallback : public RCCallback {

  public:
    TemplateCallback();
    virtual ~TemplateCallback() throw();

  public:
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual void pause() throw(RCHandlerException);
    virtual void resume(int subno) throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    virtual void timeout(NSMCommunicator& com) throw();

  };

}

#endif
