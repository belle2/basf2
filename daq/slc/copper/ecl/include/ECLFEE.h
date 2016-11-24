#ifndef _Belle2_ECLFEE_h
#define _Belle2_ECLFEE_h

#include <daq/slc/copper/FEE.h>

namespace Belle2 {

  class ECLFEE : public FEE {

  public:
    ECLFEE();
    virtual ~ECLFEE() throw() {}

  public:
    virtual void init(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void boot(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void load(RCCallback& callback, HSLB& hslb, const DBObject& obj);

  };

}

#endif
