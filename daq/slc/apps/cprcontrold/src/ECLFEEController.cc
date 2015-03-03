#include "daq/slc/apps/cprcontrold/ECLFEEController.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <mgt/hsreg.h>

#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>

extern "C" {
  void* getECLFEE()
  {
    return new Belle2::ECLFEEController();
  }
}

const int HSLB_FIRMWARE_VERSION = 0xA;
const int HSLB_HARDWARE_VERSION = 0xA;

using namespace Belle2;

bool ECLFEEController::boot(HSLBController& hslb, FEEConfig&)
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

bool ECLFEEController::load(HSLBController& hslb,
                            FEEConfig& conf)
{
  if (!FEEController::load(hslb, conf)) return false;
  hslb.writefee(0x30, 0x0d);
  LogFile::debug("write address %d (val=%d)", 0x30, 0x0d);
  hslb.writefee(0x30, 0x09);
  LogFile::debug("write address %d (val=%d)", 0x30, 0x09);
  hslb.writefee(0x30, 0x03);
  LogFile::debug("write address %d (val=%d)", 0x30, 0x03);
  return true;
}

bool ECLFEEController::monitor(HSLBController& /*hslb*/,
                               FEEConfig& /*conf*/)
{
  return true;
}

