#ifndef _Belle2_ECLFEEController_h
#define _Belle2_ECLFEEController_h

#include <daq/slc/base/IOException.h>

#include <daq/slc/apps/cprcontrold/FEEController.h>

namespace Belle2 {

  class ECLFEEController : public FEEController {

  public:
    ECLFEEController() {}
    virtual ~ECLFEEController() throw() {}

  public:
    virtual bool boot(HSLBController& hslb, FEEConfig& conf);
    virtual bool load(HSLBController& hslb, FEEConfig& conf);
    virtual bool monitor(HSLBController& hslb, FEEConfig& conf);

  };

}

#endif
