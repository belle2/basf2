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
    std::string cmd = StringUtil::form("ssh ropc01 \"cd ~b2daq/run/cdc/; sh impact-batch.sh %s\"",
                                       firmware);
    system(cmd.c_str());
  } else {
    LogFile::debug("CDC FEE firmware not exists : %s", firmware);
  }
  std::string s;
  if ((s = hslb.checkfee()) != "UNKNOWN") {
    StringList ss = StringUtil::split(s, '\n');
    for (size_t i = 0; i < ss.size(); i++) {
      LogFile::info(ss[i]);
    }
  } else {
    LogFile::error("Check FEE error");
  }
}

void CDCFEE::load(HSLB& hslb, const FEEConfig& conf)
{
  hslb.writefn(HSREG_CSR,     0x05); // reset read fifo
  hslb.writefn(HSREG_CSR,     0x06); // reset read ack
  //hslb.writefn(HSREG_CDCCONT, 0x07); // suppress mode
  hslb.writefn(HSREG_CDCCONT, 0x08); // raw mode
  hslb.writefn(HSREG_CSR,     0x0a); // parameter write

  // writing parameters to registers
  const FEEConfig::RegList& regs(conf.getRegList());
  for (FEEConfig::RegList::const_iterator it = regs.begin();
       it != regs.end(); it++) {
    const FEEConfig::Reg& reg(*it);
    if (reg.val < 0) continue;
    LogFile::debug("write address %s(%d) (val=%d, size = %d)",
                   reg.name.c_str(), reg.adr, reg.val, reg.size);
    if (reg.size == 1) {
      hslb.writefee8(reg.adr, reg.val);
    } else if (reg.size == 2) {
      hslb.writefee16(reg.adr, reg.val);
    }
  }
}

extern "C" {
  void* getCDCFEE()
  {
    return new Belle2::CDCFEE();
  }
}
