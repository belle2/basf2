#ifndef _Belle2_HVCallback_hh
#define _Belle2_HVCallback_hh

#include "daq/slc/apps/hvcontrold/HVNodeInfo.h"

#include <daq/slc/nsm/NSMCallback.h>

#include <vector>

namespace Belle2 {

  class HVCallback : public NSMCallback {

  public:
    HVCallback(NSMNode* node) throw();
    virtual ~HVCallback() throw() {}

  public:
    virtual bool turnon() throw() { return true; }
    virtual bool turnoff() throw() { return true; }
    virtual bool rampup() throw() { return true; }
    virtual bool rampdown() throw() { return true; }
    virtual bool standby() throw() { return true; }
    virtual bool standby2() throw() { return true; }
    virtual bool standby3() throw() { return true; }
    virtual bool peak() throw() { return true; }
    virtual bool recover() throw() { return true; }
    virtual bool config() throw() { return true; }
    virtual bool save() throw() { return true; }

  protected:
    virtual bool perform(NSMMessage& msg) throw(NSMHandlerException);

  };

};

#endif
