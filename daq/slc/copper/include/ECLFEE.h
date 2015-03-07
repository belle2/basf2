#ifndef _Belle2_ECLFEE_h
#define _Belle2_ECLFEE_h

#include <daq/slc/base/IOException.h>

#include <daq/slc/copper/FEE.h>

namespace Belle2 {

  class ECLFEE : public FEE {

  public:
    ECLFEE() {}
    virtual ~ECLFEE() throw() {}

  public:
    virtual bool boot(HSLB& hslb, const FEEConfig& conf);
    virtual bool load(HSLB& hslb, const FEEConfig& conf);

  };

}

#endif
