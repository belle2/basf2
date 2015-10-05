#ifndef _Belle2_RCTemplateCallback_h
#define _Belle2_RCTemplateCallback_h

#include "daq/slc/runcontrol/RCCallback.h"

namespace Belle2 {

  class RCTemplateCallback : public RCCallback {

  public:
    RCTemplateCallback();
    virtual ~RCTemplateCallback() throw();

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

  };

}

#endif
