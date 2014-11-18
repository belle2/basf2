#ifndef _Belle2_FEEController_h
#define _Belle2_FEEController_h

#include <daq/slc/apps/cprcontrold/FEEConfig.h>
#include <daq/slc/apps/cprcontrold/HSLBController.h>

namespace Belle2 {

  class FEEController {

  public:
    FEEController() {}
    virtual ~FEEController() throw() {}

  public:
    virtual bool boot(HSLBController& hslb, FEEConfig& conf) throw(IOException) = 0;
    virtual bool load(HSLBController& hslb, FEEConfig& conf) throw(IOException) = 0;
    virtual bool monitor(HSLBController& hslb, FEEConfig& conf) throw(IOException) = 0;

  };

}

#endif
