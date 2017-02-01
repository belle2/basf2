#ifndef _Belle2_TEMPLATEFEE_h
#define _Belle2_TEMPLATEFEE_h

#include <daq/slc/copper/FEE.h>

namespace Belle2 {

  class TEMPLATEFEE : public FEE {

  public:
    TEMPLATEFEE();
    virtual ~TEMPLATEFEE() throw() {}

  public:
    virtual void init(RCCallback& callback, HSLB& hslb);
    virtual void boot(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void load(RCCallback& callback, HSLB& hslb, const DBObject& obj);

  };

}

#endif
