#include "daq/slc/copper/ecl/ECLFEE.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#include <unistd.h>

const int HSLB_FIRMWARE_VERSION = 0xA;
const int HSLB_HARDWARE_VERSION = 0xA;

using namespace Belle2;

ECLFEE::ECLFEE()
{
}

void ECLFEE::boot(HSLB& hslb,  const FEEConfig& conf)
{
  hslb.writefn(0x30, 0);
  for (int ntry = 0; !hslb.checkfee(); ntry++) {
    hslb.writefn32(0x82, 0x1000);
    usleep(100);
    hslb.writefn32(0x82, 0x10);
    if (ntry > 100) {
      throw (IOException("Can not establish b2link at HSLB %c",
                         (char)('a' + hslb.get_finid())));
    }
    usleep(100);
  }
  int ver;
  if ((ver = hslb.readfn(HSREG_HWVER)) != HSLB_HARDWARE_VERSION) {
    throw (IOException("Inconsitent HWVER (%d!=%d)",
                       ver, HSLB_HARDWARE_VERSION));
  }
  if ((ver = hslb.readfn(HSREG_FWVER)) != HSLB_FIRMWARE_VERSION) {
    throw (IOException("Inconsitent FWVER (%d!=%d)",
                       ver, HSLB_FIRMWARE_VERSION));
  }
}

void ECLFEE::load(HSLB& hslb, const FEEConfig& conf)
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
  hslb.writefee(0x30, 0x0d);
  LogFile::debug("write address %d (val=%d)", 0x30, 0x0d);
  hslb.writefee(0x30, 0x09);
  LogFile::debug("write address %d (val=%d)", 0x30, 0x09);
  hslb.writefee(0x30, 0x03);
  LogFile::debug("write address %d (val=%d)", 0x30, 0x03);
}

extern "C" {
  void* getECLFEE()
  {
    return new Belle2::ECLFEE();
  }
}
