#include "daq/slc/copper/FEE.h"

#include <daq/slc/system/LogFile.h>

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

bool FEE::boot(HSLB& hslb, const FEEConfig& conf)
{
  return true;
}

bool FEE::load(HSLB& hslb, const FEEConfig& conf)
{
  const FEEConfig::RegList& regs(conf.getRegList());
  for (FEEConfig::RegList::const_iterator it = regs.begin();
       it != regs.end(); it++) {
    const FEEConfig::Reg& reg(*it);
    if (reg.size == 1) {
      hslb.writefee8(reg.adr, reg.val);
    } else if (reg.size == 4) {
      hslb.writefee32(reg.adr, reg.val);
    }
    LogFile::debug("write address %s(%d) (val=%d, size = %d)",
                   reg.name.c_str(), reg.adr, reg.val, reg.size);
  }
  return true;
}

