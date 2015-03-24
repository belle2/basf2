#ifndef _Belle2_TEMPLATEFEE_h
#define _Belle2_TEMPLATEFEE_h

#include <daq/slc/copper/FEE.h>

namespace Belle2 {

  class TEMPLATEFEE : public FEE {

  public:
    TEMPLATEFEE();
    virtual ~TEMPLATEFEE() throw() {}

  public:
    virtual void boot(HSLB& hslb, const FEEConfig& conf);
    virtual void load(HSLB& hslb, const FEEConfig& conf);

  };

}

#endif
