#include "daq/slc/copper/ecl/ECLFEE.h"
#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <mgt/hsreg.h>

#include <unistd.h>

const int HSLB_FIRMWARE_VERSION = 0xA;
const int HSLB_HARDWARE_VERSION = 0xA;

using namespace Belle2;

ECLFEE::ECLFEE()
{
}

void ECLFEE::init(RCCallback& callback, HSLB& hslb)
{
  std::string vname = StringUtil::form("ecl[%d].", hslb.get_finid());
  callback.add(new FEE32Handler(vname + "reg_num_wdata", callback, hslb, *this, 0xB0));
  callback.add(new FEE32Handler(vname + "sh_mask", callback, hslb, *this, 0xB2));
  callback.add(new FEE8Handler(vname + "shaper_mask_low", callback, hslb, *this, 0x20));
  callback.add(new FEE8Handler(vname + "shaper_mask_high", callback, hslb, *this, 0x21));
  callback.add(new FEE8Handler(vname + "ttd_trg_rare_factor", callback, hslb, *this, 0x38));
  callback.add(new FEE8Handler(vname + "ttd_trg_type", callback, hslb, *this, 0x39));
  callback.add(new FEE8Handler(vname + "calib_ampl0_low", callback, hslb, *this, 0x40));
  callback.add(new FEE8Handler(vname + "calib_ampl0_high", callback, hslb, *this, 0x41));
  callback.add(new FEE8Handler(vname + "calib_ampl_step_low", callback, hslb, *this, 0x42));
  callback.add(new FEE8Handler(vname + "calib_ampl_step_high", callback, hslb, *this, 0x43));
  callback.add(new FEE8Handler(vname + "calib_delay0_low", callback, hslb, *this, 0x44));
  callback.add(new FEE8Handler(vname + "calib_delay0_high", callback, hslb, *this, 0x45));
  callback.add(new FEE8Handler(vname + "calib_delay_step_low", callback, hslb, *this, 0x46));
  callback.add(new FEE8Handler(vname + "calib_delay_step_high", callback, hslb, *this, 0x47));
  callback.add(new FEE8Handler(vname + "calib_event_per_step", callback, hslb, *this, 0x48));
}

void ECLFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  /*
  int ver;
  if ((ver = hslb.readfee8(HSREG_HWVER)) != HSLB_HARDWARE_VERSION) {
    throw (IOException("Inconsitent HWVER (%d!=%d)",
                       ver, HSLB_HARDWARE_VERSION));
  }
  if ((ver = hslb.readfee8(HSREG_FWVER)) != HSLB_FIRMWARE_VERSION) {
    throw (IOException("Inconsitent FWVER (%d!=%d)",
                       ver, HSLB_FIRMWARE_VERSION));
  }
  hslb.writefee8(0x30, 0x00);
  */
}

void ECLFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  hslb.writefee32(0x00B0, (obj.getInt("reg_num") << 16) & obj.getInt("reg_wdata"));
  hslb.writefee32(0x00B2, obj.getInt("sh_mask"));
  hslb.writefee32(0x00B8, 0x4);
  int er_code = 1;
  for (int i = 0; i < 1000; i++) {
    er_code = hslb.readfee32(0x00B8);
    if (er_code == 00000000) break;
    usleep(10);
  }
  if (er_code != 00000000) {
    LogFile::error("Failed to set sh_wreg");
  }

  hslb.writefee8(0x20, obj.getInt("shaper_mask_low"));
  hslb.writefee8(0x21, obj.getInt("shaper_mask_high"));
  hslb.writefee8(0x38, obj.getInt("ttd_trg_rare_factor"));
  hslb.writefee8(0x39, obj.getInt("ttd_trg_type"));
  hslb.writefee8(0x40, obj.getInt("calib_ampl0_low"));
  hslb.writefee8(0x41, obj.getInt("calib_ampl0_high"));
  hslb.writefee8(0x42, obj.getInt("calib_ampl_step_high"));
  hslb.writefee8(0x43, obj.getInt("calib_ampl_step_high"));

  hslb.writefee8(0x45, obj.getInt("calib_delay0_high"));
  hslb.writefee8(0x46, obj.getInt("calib_delay_step_low"));
  hslb.writefee8(0x47, obj.getInt("calib_delay_step_high"));
  hslb.writefee8(0x48, obj.getInt("calib_events_per_step"));

  hslb.writefee8(0x30, 0x0D);
  hslb.writefee8(0x30, 0x09);
}

extern "C" {
  void* getECLFEE()
  {
    return new Belle2::ECLFEE();
  }
}
