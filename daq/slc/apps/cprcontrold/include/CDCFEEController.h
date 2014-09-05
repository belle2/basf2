#ifndef _Belle2_CDCFEEController_h
#define _Belle2_CDCFEEController_h

#include <daq/slc/apps/cprcontrold/FEEController.h>

namespace Belle2 {

  class CDCFEEController : public FEEController {

  public:
    CDCFEEController();
    virtual ~CDCFEEController() throw() {}

  public:
    virtual bool boot(HSLBController& hslb, FEEConfig& conf) throw();
    virtual bool load(HSLBController& hslb, FEEConfig& conf) throw();
    virtual bool monitor(HSLBController& hslb, FEEConfig& conf) throw();

  };

}

#endif
