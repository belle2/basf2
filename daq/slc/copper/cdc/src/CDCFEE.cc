#include "daq/slc/copper/cdc/CDCFEE.h"
#include "daq/slc/copper/cdc/CDCFEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <cstdlib>

#include <mgt/hsreg.h>

using namespace Belle2;

CDCFEE::CDCFEE()
{
}

void CDCFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& /*obj*/)
{
  std::string vname = StringUtil::form("cdc[%d]", hslb.get_finid());
  LogFile::warning(vname);
  callback.add(new CDCDateHandler(vname + ".date", callback, hslb, *this));
  callback.add(new CDCFirmwareHandler(vname + ".fmver", callback, hslb, *this));
  callback.add(new CDCDataFormatHandler(vname + ".mode", callback, hslb, *this));
  callback.add(new CDCWindowHandler(vname + ".window", callback, hslb, *this));
  callback.add(new CDCDelayHandler(vname + ".delay", callback, hslb, *this));
  callback.add(new CDCADCThresholdHandler(vname + ".adcth", callback, hslb, *this));
  callback.add(new CDCTDCThresholdHandler(vname + ".tdcth", callback, hslb, *this));
  callback.add(new CDCIndirectADCAccessHandler(vname + ".indirectadc", callback, hslb, *this));
  callback.add(new CDCDACControlHandler(vname + ".dac", callback, hslb, *this));
  callback.add(new CDCIndirectMonitorAccessHandler(vname + ".indirectmon", callback, hslb, *this));
  for (int i = 0; i < 48; i++) {
    std::string vname = StringUtil::form("cdc[%d].ped[%d]", hslb.get_finid(), i);
    callback.add(new CDCPedestalHandler(vname, callback, hslb, *this, i));
  }
  callback.add(new NSMVHandlerFloat(vname + ".tem", true, false, 0));
  callback.add(new NSMVHandlerFloat(vname + ".vccint", true, false, 0));
  callback.add(new NSMVHandlerFloat(vname + ".vccaux", true, false, 0));
  callback.add(new NSMVHandlerFloat(vname + ".vcc5v", true, false, 0));
}

void CDCFEE::boot(RCCallback& callback, HSLB& hslb,  const DBObject& obj)
{
  const std::string firmware = obj.getText("firm");
  /*
  if (File::exist(firmware)) {
    LogFile::info("Loading CDC FEE firmware: %s", firmware.c_str());
    std::string cmd = "ssh ropc01 \"cd ~b2daq/run/cdc/; sh impact-batch.sh " +
                      firmware + "\"";
    system(cmd.c_str());
  } else {
    LogFile::debug("CDC FEE firmware not exists : %s", firmware.c_str());
  }
  */
  int err = 0;
  if ((err = hslb.test()) > 0) {
    LogFile::error("Test HSLB failed");
  }
}

void CDCFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  // setting CDC control (data format, window and delay)
  int val = 0;
  std::string mode = StringUtil::tolower(obj.getText("mode"));
  if (mode == "suppress") {
    LogFile::info("suppress mode");
    val = 2 << 24;
  } else if (mode == "raw") {
    LogFile::info("raw mode");
    val = 1 << 24;
  } else if (mode == "raw-suppress") {
    LogFile::info("raw-suppress mode");
    val = 3 << 24;
  } else {
    LogFile::warning("no readout");
    val = 0;
  }
  val |= (obj("window").getInt("val") & 0xFF) << 8;
  val |= obj("delay").getInt("val") & 0xFF;
  hslb.writefee32(0x0012, val);

  // setting ADC threshold
  val = obj("adcth").getInt("val") & 0xFFFF;
  hslb.writefee32(0x0013, val);

  // setting DAC control
  val = obj("tdcth").getInt("val") & 0x7FFF;
  hslb.writefee32(0x0015, val);

  // setting Pedestals
  const DBObjectList o_ped(obj.getObjects("ped"));
  for (size_t i = 0; i < o_ped.size() / 2; i++) {
    val = (o_ped[2 * i].getInt("val") & 0xFFFF)
          | ((o_ped[2 * i + 1].getInt("val") << 16) & 0xFFFF0000);
    hslb.writefee32(0x0020 + i, val);
  }
}

void CDCFEE::monitor(RCCallback& callback, HSLB& hslb)
{
  std::string vname = StringUtil::form("cdc[%d]", hslb.get_finid());

  // System monitor
  hslb.writefee32(0X0016, 0x00480F00);
  callback.wait(0.5);
  hslb.writefee32(0X0016, 0x00400400);
  callback.wait(0.5);
  hslb.writefee32(0X0016, 0x004120F0);
  callback.wait(0.5);

  // temparature
  hslb.writefee32(0X0016, 0x80000000);
  callback.wait(0.5);
  int ret = hslb.readfee32(0X0016);
  int v0 = ret & 0xffff;
  float tem = (v0 / 64.) * (503.975 / 1024.) - 273.15;
  callback.set(vname + ".tem", tem);

  // VCCINT
  hslb.writefee32(0X0016, 0x80010000);
  callback.wait(0.5);
  ret = hslb.readfee32(0X0016);
  v0 = ret & 0xffff;
  float vccint = (v0 / 64.) * (3.0 / 1024.);
  callback.set(vname + ".vccint", vccint);

  // VCCAUX
  hslb.writefee32(0X0016, 0x80020000);
  callback.wait(0.5);
  ret = hslb.readfee32(0X0016);
  v0 = ret & 0xffff;
  float vccaux = (v0 / 64.) * (3.0 / 1024.);
  callback.set(vname + ".vccaux", vccaux);

  // VCC5V
  hslb.writefee32(0X0016, 0x80030000);
  callback.wait(0.5);
  ret = hslb.readfee32(0X0016);
  v0 = ret & 0xffff;
  float vcc5v = ((390. + 51. + 39.) * v0 / (64.*1024.)) / 51.;
  callback.set(vname + ".vcc5v", vcc5v);

}

extern "C" {
  void* getCDCFEE()
  {
    return new Belle2::CDCFEE();
  }
}
