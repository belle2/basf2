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
  private:
    void rio_sh_wreg(RCCallback& callback, HSLB& hslb, unsigned int sh_num,
                     unsigned int reg_num, unsigned int reg_wdata);

  };

}

#endif
