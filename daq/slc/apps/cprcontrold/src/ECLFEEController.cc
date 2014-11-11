#include "daq/slc/apps/cprcontrold/ECLFEEController.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <mgt/hsreg.h>

#include <unistd.h>
#include <cstdlib>
#include <iostream>

using namespace Belle2;

ECLFEEController::ECLFEEController()
{
  LogFile::debug("ECLFEEController");
}

bool ECLFEEController::boot(HSLBController& hslb,
                            FEEConfig& /*conf*/) throw()
{
  hslb.writefn(0x30, 0);
  return true;
}

bool ECLFEEController::load(HSLBController& hslb,
                            FEEConfig& /*conf*/) throw()
{
  std::vector<Reg> reg_v;
  reg_v.push_back(Reg(0x20, 0xFF, "SHAPER_MASK_LOW"));
  reg_v.push_back(Reg(0x21, 0x0F, "SHAPER_MASK_HIGH"));
  reg_v.push_back(Reg(0x38, 0xc1, "TTD_TRG_RARE_FACTOR"));
  reg_v.push_back(Reg(0x39, 0x11, "TTD_TRG_TYPE"));
  reg_v.push_back(Reg(0x40, 0x00, "CALIB_AMPL0_LOW"));
  reg_v.push_back(Reg(0x41, 0x00, "CALIB_AMPL0_HIGH"));
  reg_v.push_back(Reg(0x42, 0x00, "CALIB_AMPL_STEP_HIGH"));
  reg_v.push_back(Reg(0x43, 0x00, "CALIB_AMPL_STEP_HIGH"));
  reg_v.push_back(Reg(0x44, 0x00, "CALIB_DELAY0_LOW"));
  reg_v.push_back(Reg(0x45, 0x00, "CALIB_DELAY0_HIGH"));
  reg_v.push_back(Reg(0x46, 0x00, "CALIB_DELAY_STEP_LOW"));
  reg_v.push_back(Reg(0x47, 0x00, "CALIB_DELAY_STEP_HIGH"));
  reg_v.push_back(Reg(0x48, 0x00, "CALIB_EVENTS_PER_STEP"));
  reg_v.push_back(Reg(0x30, 0x0D, ""));
  reg_v.push_back(Reg(0x30, 0x09, ""));
  for (std::vector<Reg>::iterator it = reg_v.begin();
       it != reg_v.end(); it++) {
    Reg& reg(*it);
    std::string cmd = StringUtil::form("rio_reg_io w %c %x %x",
                                       (hslb.get_finid() + 'a'), reg.adr, reg.val);
    system(cmd.c_str());
    LogFile::debug(cmd);
  }
  /*
  FEEConfig::ParameterList& pars(conf.getParameters());
  for (FEEConfig::ParameterList::iterator it = pars.begin();
       it != pars.end(); it++) {
    FEEConfig::Parameter& par(*it);
    FEEConfig::Register& reg(*conf.getRegister(par.getName()));
    int addr = reg.getAddress() + par.getIndex() * 2;
    int val = par.getValue();
    hslb.writefee(addr, val);
    LogFile::debug("write address %d (val=%d)", addr, val);
  }
  hslb.writefee(0x30, 0x0d);
  hslb.writefee(0x30, 0x09);
  hslb.writefee(0x30, 0x03);
  */

  return true;
}

bool ECLFEEController::monitor(HSLBController& /*hslb*/,
                               FEEConfig& /*conf*/) throw()
{
  return true;
}

extern "C" {
  void* getECLFEE()
  {
    return new Belle2::ECLFEEController();
  }
}
