#include "daq/slc/apps/cprcontrold/ECLFEEController.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#include <unistd.h>
#include <cstdlib>
#include <iostream>

using namespace Belle2;

ECLFEEController::ECLFEEController()
{
  LogFile::debug("ECLFEEController");
}

bool ECLFEEController::boot(HSLBController& /*hslb*/,
                            FEEConfig& /*conf*/) throw()
{

  return true;
}

bool ECLFEEController::load(HSLBController& hslb,
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

bool ECLFEEController::monitor(HSLBController& /*hslb*/,
                               FEEConfig& /*conf*/) throw()
{
  return true;
}

extern "C" {
  void* getECLFEE()
  {
    return new Belle2::ECLFEEController();
  }
}
