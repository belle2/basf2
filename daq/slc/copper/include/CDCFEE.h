#ifndef _Belle2_CDCFEE_h
#define _Belle2_CDCFEE_h

#include <daq/slc/copper/FEE.h>

namespace Belle2 {

  class CDCFEE : public FEE {

  public:
    CDCFEE();
    virtual ~CDCFEE() throw() {}

  public:
    virtual bool boot(HSLB& hslb, const FEEConfig& conf);
    virtual bool load(HSLB& hslb, const FEEConfig& conf);

  };

}

#endif
