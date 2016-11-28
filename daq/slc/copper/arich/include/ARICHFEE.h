#ifndef _Belle2_ARICHFEE_h
#define _Belle2_ARICHFEE_h

#include <daq/slc/copper/FEE.h>

namespace Belle2 {

  class N6SA0xHSLB;

  class ARICHFEE : public FEE {

  public:
    ARICHFEE();
    virtual ~ARICHFEE() throw() {}

  public:
    virtual void init(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void boot(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void load(RCCallback& callback, HSLB& hslb, const DBObject& obj);

  private:
    void write_read(HSLB& hslb, int adr, int val) throw(HSLBHandlerException);

  };

}

#endif
