#ifndef _Belle2_TRGFEE_h
#define _Belle2_TRGFEE_h

#include <daq/slc/copper/FEE.h>

namespace Belle2 {

  class TRGFEE : public FEE {

  public:
    TRGFEE();
    virtual ~TRGFEE() throw() {}

  public:
    virtual void init(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void boot(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void load(RCCallback& callback, HSLB& hslb, const DBObject& obj);

  };

}

#endif
