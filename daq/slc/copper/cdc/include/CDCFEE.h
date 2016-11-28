#ifndef _Belle2_CDCFEE_h
#define _Belle2_CDCFEE_h

#include <daq/slc/copper/FEE.h>

namespace Belle2 {

  class CDCFEE : public FEE {

  public:
    CDCFEE();
    virtual ~CDCFEE() throw() {}

  public:
    virtual void init(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void boot(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void load(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void monitor(RCCallback& callback, HSLB& hslb);

  };

}

#endif
