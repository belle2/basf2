#ifndef _Belle2_DUMMYFEE_h
#define _Belle2_DUMMYFEE_h

#include <daq/slc/copper/FEE.h>

namespace Belle2 {

  class DUMMYFEE : public FEE {

  public:
    DUMMYFEE();
    virtual ~DUMMYFEE() throw() {}

  public:
    virtual void init(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void boot(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void load(RCCallback& callback, HSLB& hslb, const DBObject& obj);

  };

}

#endif
