#include "daq/slc/copper/template/TEMPLATEFEE.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

using namespace Belle2;

TEMPLATEFEE::TEMPLATEFEE()
{
}

void TEMPLATEFEE::init(RCCallback& callback, HSLB& hslb)
{

}

void TEMPLATEFEE::boot(HSLB& hslb,  const FEEConfig& conf)
{

}

void TEMPLATEFEE::load(HSLB& hslb, const FEEConfig& conf)
{
  // file streaming
  const char* stream = conf.getStream();
  if (stream && File::exist(stream)) {
    hslb.writestream(stream);
  }
  // writing parameters to registers
  const FEEConfig::RegList& regs(conf.getRegList());
  for (FEEConfig::RegList::const_iterator it = regs.begin();
       it != regs.end(); it++) {
    const FEEConfig::Reg& reg(*it);
    LogFile::debug("write address %s(%d) (val=%d, size = %d)",
                   reg.name.c_str(), reg.adr, reg.val, reg.size);
    if (reg.size == 1) {
      hslb.writefee8(reg.adr, reg.val);
    } else if (reg.size == 4) {
      hslb.writefee32(reg.adr, reg.val);
    }
  }
}

extern "C" {
  void* getTEMPLATEFEE()
  {
    return new Belle2::TEMPLATEFEE();
  }
}
