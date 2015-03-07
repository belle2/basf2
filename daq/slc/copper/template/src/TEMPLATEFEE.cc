#include "daq/slc/copper/template/TEMPLATEFEE.h"

#include <mgt/hsreg.h>

using namespace Belle2;

TEMPLATEFEE::TEMPLATEFEE()
{
}

bool TEMPLATEFEE::boot(HSLB& hslb,  const FEEConfig& conf)
{
  return FEE::boot(hslb, conf);
}

bool TEMPLATEFEE::load(HSLB& hslb, const FEEConfig& conf)
{
  return FEE::load(hslb, conf);
}

extern "C" {
  void* getTEMPLATEFEE()
  {
    return new Belle2::TEMPLATEFEE();
  }
}
