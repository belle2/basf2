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

bool ECLFEEController::boot(HSLBController& hslb,
                            FEEConfig& /*conf*/) throw()
{
  hslb.writefn(0x30, 0);
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
    hslb.writefee(addr, val);
    LogFile::debug("write address %d (val=%d)", addr, val);
  }
  hslb.writefee(0x30, 0x0d);
  hslb.writefee(0x30, 0x09);
  hslb.writefee(0x30, 0x03);
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
