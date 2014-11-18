#include "daq/slc/apps/cprcontrold/ECLFEEController.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <mgt/hsreg.h>

#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#define HSLB_FIRMWARE_VERSION  (0xA)
#define HSLB_HARDWARE_VERSION  (0xA)

using namespace Belle2;

ECLFEEController::ECLFEEController()
{
  LogFile::debug("ECLFEEController");
}

bool ECLFEEController::boot(HSLBController& hslb,
                            FEEConfig& /*conf*/) throw(IOException)
{
  hslb.writefn(0x30, 0);
  return true;
}

bool ECLFEEController::load(HSLBController& hslb,
                            FEEConfig& conf) throw(IOException)
{
  int ret;
  if ((ret = hslb.readfn(HSREG_HWVER)) != HSLB_FIRMWARE_VERSION) {
    throw (IOException(1, "HLSB Firmware version %.4x is not suitable for this applicaton", ret));
  }
  if ((ret = hslb.readfn(HSREG_FWVER)) != HSLB_HARDWARE_VERSION) {
    throw (IOException(2, "HLSB Hardware version %.4x is not suitable for this applicaton", ret));
  }
  FEEConfig::ParameterList& pars(conf.getParameters());
  for (FEEConfig::ParameterList::iterator it = pars.begin();
       it != pars.end(); it++) {
    FEEConfig::Parameter& par(*it);
    FEEConfig::Register& reg(*conf.getRegister(par.getName()));
    int adr = reg.getAddress();
    int val = par.getValue();
    hslb.writefee(adr, val);
    LogFile::debug("write address %d (val=%d)", adr, val);
    /*
    std::string cmd = StringUtil::form("rio_reg_io w %c %x %x",
                                       (hslb.get_finid() + 'a'), adr, val);
    system(cmd.c_str());
    LogFile::debug(cmd);
    */
  }
  hslb.writefee(0x30, 0x0d);
  LogFile::debug("write address %d (val=%d)", 0x30, 0x0d);
  hslb.writefee(0x30, 0x09);
  LogFile::debug("write address %d (val=%d)", 0x30, 0x09);
  hslb.writefee(0x30, 0x03);
  LogFile::debug("write address %d (val=%d)", 0x30, 0x03);

  return true;
}

bool ECLFEEController::monitor(HSLBController& /*hslb*/,
                               FEEConfig& /*conf*/) throw(IOException)
{
  return true;
}

int ECLFEEController::rio_reg_io(HSLBController& hslb, const char* opt,
                                 unsigned int reg_num, unsigned int reg_wdata)
throw(IOException)
{
  int fin_num = hslb.get_finid();
  hslb.writefn(0x30, 0x0d);
  if (strcmp(opt, "r") == 0) { //read
    int val = hslb.readfee(reg_num);
    if (val != 0x11) {
      throw (IOException(3, "incorrct status: 0x%02X\n", val));
    }
    printf("HSLB%c[0x%02X] => 0x%02X\n", fin_num + 'a', reg_num, val);
    return val;
  } else if (strcmp(opt, "w") == 0) { //write
    if (hslb.writefee(reg_num, reg_wdata) < 0) {
      throw (IOException("Failed to write to fee via HSLB %c", fin_num + 'a'));
      return 0;
    }
    printf("HSLB%c[0x%02X] <= 0x%02X\n", fin_num + 'a', reg_num, reg_wdata);
    return 1;
  }
  return 0;
}

extern "C" {
  void* getECLFEE()
  {
    return new Belle2::ECLFEEController();
  }
}

