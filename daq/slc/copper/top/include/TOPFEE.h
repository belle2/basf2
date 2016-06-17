#ifndef _Belle2_TOPFEE_h
#define _Belle2_TOPFEE_h

#include <daq/slc/copper/FEE.h>

namespace Belle2 {

  class TOPFEE : public FEE {

  public:
    TOPFEE();
    virtual ~TOPFEE() throw() {}

  public:
    virtual void init(RCCallback& callback, HSLB& hslb);
    virtual void boot(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void load(RCCallback& callback, HSLB& hslb, const DBObject& obj);

  };

}

#endif
