#include "daq/slc/copper/cdc/CDCFEE.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <cstdlib>

#include <mgt/hsreg.h>

using namespace Belle2;

CDCFEE::CDCFEE()
{
}

void CDCFEE::boot(HSLB& hslb,  const FEEConfig& conf)
{
  const char* firmware = conf.getFirmware();
  if (firmware && File::exist(firmware)) {
    LogFile::info("Loading CDC FEE firmware: %s", firmware);
    std::string cmd = StringUtil::form("cd ~/run/cdc/; sh impact-batch.sh %s",
                                       firmware);
    system(cmd.c_str());
  }
}

void CDCFEE::load(HSLB& hslb, const FEEConfig& conf)
{
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
  void* getCDCFEE()
  {
    return new Belle2::CDCFEE();
  }
}
