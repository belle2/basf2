#include "daq/slc/copper/CDCFEE.h"

#include <mgt/hsreg.h>

using namespace Belle2;

CDCFEE::CDCFEE()
{
}

bool CDCFEE::boot(HSLB& /*hslb*/,  const FEEConfig& /*conf*/)
{
  return true;
}

bool CDCFEE::load(HSLB& hslb, const FEEConfig& conf)
{
  const FEEConfig::RegList& regs(conf.getRegList());
  for (FEEConfig::RegList::const_iterator it = regs.begin();
       it != regs.end(); it++) {
    const FEEConfig::Reg& reg(*it);
    int addr = reg.adr;
    int val = reg.val;
    hslb.writefn(HSREG_CSR,     0x05); /* reset read fifo */
    hslb.writefn(addr + 0, (val >> 0) & 0xff);
    hslb.writefn(addr + 1, (val >> 8) & 0xff);
    hslb.writefn(HSREG_CSR,     0x0a); /* parameter write */
  }
  return true;
}

extern "C" {
  void* getCDCFEE()
  {
    return new Belle2::CDCFEE();
  }
}
