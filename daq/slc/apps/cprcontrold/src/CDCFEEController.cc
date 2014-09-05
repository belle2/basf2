#include "daq/slc/apps/cprcontrold/CDCFEEController.h"

#include <mgt/hsreg.h>

#include <unistd.h>
#include <cstdlib>
#include <iostream>

using namespace Belle2;

CDCFEEController::CDCFEEController()
{
  std::cout << "CDCFEEController" << std::endl;
}

bool CDCFEEController::boot(HSLBController& /*hslb*/,
                            FEEConfig& /*conf*/) throw()
{
  return true;
}

bool CDCFEEController::load(HSLBController& hslb,
                            FEEConfig& conf) throw()
{
  FEEConfig::ParameterList& pars(conf.getParameters());
  for (FEEConfig::ParameterList::iterator it = pars.begin();
       it != pars.end(); it++) {
    FEEConfig::Parameter& par(*it);
    FEEConfig::Register& reg(*conf.getRegister(par.getName()));
    int addr = reg.getAddress() + par.getIndex() * 2;
    int val = par.getValue();
    hslb.writefn(HSREG_CSR,     0x05); /* reset read fifo */
    hslb.writefn(addr + 0, (val >> 0) & 0xff);
    hslb.writefn(addr + 1, (val >> 8) & 0xff);
    hslb.writefn(HSREG_CSR,     0x0a); /* parameter write */
  }
  return true;
}

bool CDCFEEController::monitor(HSLBController& /*hslb*/,
                               FEEConfig& /*conf*/) throw()
{
  return true;
}

extern "C" {
  void* getCDCFEE()
  {
    return new Belle2::CDCFEEController();
  }
}
