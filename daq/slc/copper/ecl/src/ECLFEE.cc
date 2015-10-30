#include "daq/slc/copper/ecl/ECLFEE.h"
#include "daq/slc/copper/ecl/ECLFEEHandler.h"

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

void ECLFEE::init(RCCallback& callback, HSLB& hslb)
{
  std::string vname = StringUtil::form("ecl[%d]", hslb.get_finid());
  callback.add(new ECLShaperMaskLowHandler(vname + ".shaper.mask.low", callback, hslb, *this, 0x20));
  callback.add(new ECLShaperMaskHighHandler(vname + ".shaper.mask.high", callback, hslb, *this, 0x21));
  callback.add(new ECLTTTrgRareFactorHandler(vname + ".ttd.trg.rare.factor", callback, hslb, *this, 0x38));
  callback.add(new ECLTTTrgTypeHandler(vname + ".ttd.trg.type", callback, hslb, *this, 0x39));
  callback.add(new ECLCalibAmpl0LowHandler(vname + ".calib.ampl0.low", callback, hslb, *this, 0x40));
  callback.add(new ECLCalibAmpl0HighHandler(vname + ".calib.ampl0.high", callback, hslb, *this, 0x41));
  callback.add(new ECLCalibAmplStepLowHandler(vname + ".calib.ampl.step.low", callback, hslb, *this, 0x42));
  callback.add(new ECLCalibAmplStepHighHandler(vname + ".calib.ampl.step.high", callback, hslb, *this, 0x43));
  callback.add(new ECLCalibDelay0LowHandler(vname + ".calib.delay0.low", callback, hslb, *this, 0x44));
  callback.add(new ECLCalibDelay0HighHandler(vname + ".calib.delay0.high", callback, hslb, *this, 0x45));
  callback.add(new ECLCalibDelaytepLowHandler(vname + ".calib.delay.step.low", callback, hslb, *this, 0x46));
  callback.add(new ECLCalibDelayStepHighHandler(vname + ".calib.delay.step.high", callback, hslb, *this, 0x47));
  callback.add(new ECLCalibEventPerStepHandler(vname + ".calib.event.per.step", callback, hslb, *this, 0x48));
}

void ECLFEE::boot(HSLB& hslb,  const DBObject&)
{
  /*
  hslb.writefn(0x30, 0);
  for (int ntry = 0; hslb.checkfee() == "UNKNOWN"; ntry++) {
    hslb.writefn(0x82, 0x1000);
    usleep(100);
    hslb.writefn(0x82, 0x10);
    if (ntry > 100) {
      throw (IOException("Can not establish b2link at HSLB %c",
                         (char)('a' + hslb.get_finid())));
    }
    usleep(100);
  }
  */
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

void ECLFEE::load(HSLB& hslb, const DBObject& obj)
{
  // writing parameters to registers
  /*
  const DBObjectList objs(obj.getObjects("par"));
  for (DBObjectList::const_iterator it = objs.begin();
       it != objs.end(); it++) {
    const DBObject& o_par(*it);
    int adr = o_par.getInt("adr");
    int val = o_par.getInt("val");
    LogFile::debug("writefee adr=%d, val=%d", adr, val);
    hslb.writefee32(adr, val);
  }
  hslb.writefee32(0x30, 0x0d);
  LogFile::debug("writefee adr=%d, val=%d", 0x30, 0x0d);
  hslb.writefee32(0x30, 0x09);
  LogFile::debug("writefee adr=%d, val=%d", 0x30, 0x09);
  hslb.writefee32(0x30, 0x03);
  LogFile::debug("writefee adr=%d, val=%d", 0x30, 0x03);
  */

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

extern "C" {
  void* getECLFEE()
  {
    return new Belle2::ECLFEE();
  }
}
