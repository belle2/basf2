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

void ECLFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  std::string vname = StringUtil::form("ecl[%d].", hslb.get_finid());
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
  /*
  if (obj.hasValue("shaper_mask_low")) {
    hslb.writefee8(0x20, obj.getInt("shaper_mask_low"));       // SHAPER_MASK_LOW
    hslb.writefee8(0x21, obj.getInt("shaper_mask_high"));      // SHAPER_MASK_HIGH
    hslb.writefee8(0x38, obj.getInt("ttd_trg_rare_factor"));   // TTD_TRG_RARE_FACTOR
    hslb.writefee8(0x39, obj.getInt("ttd_trg_type"));          // TTD_TRG_TYPE

    hslb.writefee8(0x40, obj.getInt("calib_ampl0_low"));       // CALIB_AMPL0_LOW
    hslb.writefee8(0x41, obj.getInt("calib_ampl0_high"));      // CALIB_AMPL0_HIGH
    hslb.writefee8(0x42, obj.getInt("calib_ampl_step_high"));  // CALIB_AMPL_STEP_HIGH
    hslb.writefee8(0x43, obj.getInt("calib_ampl_step_high"));  // CALIB_AMPL_STEP_HIGH

    hslb.writefee8(0x44, obj.getInt("calib_delay0_low"));      // CALIB_DELAY0_LOW
    hslb.writefee8(0x45, obj.getInt("calib_delay0_high"));     // CALIB_DELAY0_HIGH
    hslb.writefee8(0x46, obj.getInt("calib_delay_step_low"));  // CALIB_DELAY_STEP_LOW
    hslb.writefee8(0x47, obj.getInt("calib_delay_step_high")); // CALIB_DELAY_STEP_HIGH

    hslb.writefee8(0x48, obj.getInt("calib_events_per_step")); // CALIB_EVENTS_PER_STEP

    hslb.writefee8(0x30, 0x0D);
    hslb.writefee8(0x30, 0x09);
  } else {
    const int SHAPER_MASK_LOW  = 0xFF;
    const int SHAPER_MASK_HIGH = 0x0F;
    const int TTD_TRG_RARE_FACTOR = 0xc1;
    const int TTD_TRG_TYPE        = 0x11;

    const int CALIB_AMPL0_LOW  = 0x00;
    const int CALIB_AMPL0_HIGH = 0x00;
    const int CALIB_AMPL_STEP_HIGH = 0x00;

    const int CALIB_DELAY0_LOW  = 0x00;
    const int CALIB_DELAY0_HIGH = 0x00;
    const int CALIB_DELAY_STEP_LOW  = 0x00;
    const int CALIB_DELAY_STEP_HIGH = 0x00;

    const int CALIB_EVENTS_PER_STEP = 0x00;

    hslb.writefee8(0x20, SHAPER_MASK_LOW);
    hslb.writefee8(0x21, SHAPER_MASK_HIGH);
    hslb.writefee8(0x38, TTD_TRG_RARE_FACTOR);
    hslb.writefee8(0x39, TTD_TRG_TYPE);

    hslb.writefee8(0x40, CALIB_AMPL0_LOW);
    hslb.writefee8(0x41, CALIB_AMPL0_HIGH);
    hslb.writefee8(0x42, CALIB_AMPL_STEP_HIGH);
    hslb.writefee8(0x43, CALIB_AMPL_STEP_HIGH);

    hslb.writefee8(0x44, CALIB_DELAY0_LOW);
    hslb.writefee8(0x45, CALIB_DELAY0_HIGH);
    hslb.writefee8(0x46, CALIB_DELAY_STEP_LOW);
    hslb.writefee8(0x47, CALIB_DELAY_STEP_HIGH);

    hslb.writefee8(0x48, CALIB_EVENTS_PER_STEP);

    hslb.writefee8(0x30, 0x0D);
    hslb.writefee8(0x30, 0x09);
  }

  printf("%x\n", hslb.readfee8(0x20));
  printf("%x\n", hslb.readfee8(0x21));
  printf("%x\n", hslb.readfee8(0x38));
  printf("%x\n", hslb.readfee8(0x39));

  printf("%x\n", hslb.readfee8(0x40));
  printf("%x\n", hslb.readfee8(0x41));
  printf("%x\n", hslb.readfee8(0x42));
  printf("%x\n", hslb.readfee8(0x43));

  printf("%x\n", hslb.readfee8(0x44));
  printf("%x\n", hslb.readfee8(0x45));
  printf("%x\n", hslb.readfee8(0x46));
  printf("%x\n", hslb.readfee8(0x47));

  printf("%x\n", hslb.readfee8(0x48));

  printf("%x\n", hslb.readfee8(0x30));
  printf("%x\n", hslb.readfee8(0x30));
  */
}

extern "C" {
  void* getECLFEE()
  {
    return new Belle2::ECLFEE();
  }
}
