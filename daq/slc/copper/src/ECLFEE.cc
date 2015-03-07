#include "daq/slc/copper/ECLFEE.h"

#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#include <unistd.h>

const int HSLB_FIRMWARE_VERSION = 0xA;
const int HSLB_HARDWARE_VERSION = 0xA;

using namespace Belle2;

bool ECLFEE::boot(HSLB& hslb, const FEEConfig&)
{
  hslb.writefn(0x30, 0);
  for (int ntry = 0; !hslb.checkfee(); ntry++) {
    hslb.writefn32(0x82, 0x1000);
    usleep(100);
    hslb.writefn32(0x82, 0x10);
    if (ntry > 100) {
      LogFile::error("Can not establish b2link at HSLB %c",
                     (char)('a' + hslb.get_finid()));
      return false;
    }
    usleep(100);
  }
  if (hslb.readfn(HSREG_HWVER) != HSLB_FIRMWARE_VERSION) {
    return false;
  }
  if (hslb.readfn(HSREG_FWVER) != HSLB_HARDWARE_VERSION) {
    return false;
  }
  return true;
}

bool ECLFEE::load(HSLB& hslb, const FEEConfig& conf)
{
  if (!FEE::load(hslb, conf)) return false;
  hslb.writefee(0x30, 0x0d);
  LogFile::debug("write address %d (val=%d)", 0x30, 0x0d);
  hslb.writefee(0x30, 0x09);
  LogFile::debug("write address %d (val=%d)", 0x30, 0x09);
  hslb.writefee(0x30, 0x03);
  LogFile::debug("write address %d (val=%d)", 0x30, 0x03);
  return true;
}

extern "C" {
  void* getECLFEE()
  {
    return new Belle2::ECLFEE();
  }
}

