#ifndef _Belle2_HVCallback_hh
#define _Belle2_HVCallback_hh

#include "daq/slc/hvcontrol/HVState.h"

#include <daq/slc/nsm/NSMCallback.h>

#include <vector>

namespace Belle2 {

  class HVCallback : public NSMCallback {

  public:
    HVCallback(const NSMNode& node) throw();
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
    const HVState& getStateDemand() const throw() { return _state_demand; }

  protected:
    virtual bool perform(const NSMMessage& msg) throw();

  private:
    HVState _state_demand;

  };

};

#endif
