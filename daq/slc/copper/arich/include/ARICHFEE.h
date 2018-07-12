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
    virtual void start(RCCallback& callback, HSLB& hslb);
    virtual void stop(RCCallback& callback, HSLB& hslb);
    virtual void monitor(RCCallback& callback, HSLB& hslb);
    virtual void readback(RCCallback& callback, HSLB& hslb, const DBObject& obj);

  private:
    unsigned int m_reg[256 * 256];
    double m_tstamp;
    DBObject m_o_feb[6];
    unsigned m_serial;

  };

}

#endif
