#include "daq/slc/apps/cprcontrold/FEEController.h"

#include <daq/slc/system/LogFile.h>

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

bool FEEController::load(HSLBController& hslb,
                         FEEConfig& conf)
{
  FEEConfig::ParameterList& pars(conf.getParameters());
  for (FEEConfig::ParameterList::iterator it = pars.begin();
       it != pars.end(); it++) {
    FEEConfig::Parameter& par(*it);
    FEEConfig::Register& reg(*conf.getRegister(par.getName()));
    int adr = reg.getAddress();
    int val = par.getValue();
    hslb.writefee(adr, val);
    LogFile::debug("write address %s(%d) (val=%d)",
                   par.getName().c_str(), adr, val);
  }
  return true;
}

