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

void CDCFEE::init(RCCallback& callback, HSLB& hslb)
{
  std::string vname = StringUtil::form("cdc[%d]", hslb.get_finid());
  callback.add(new CDCDateHandler(vname + ".date", callback, hslb, *this));
  callback.add(new CDCFirmwareHandler(vname + ".firmver", callback, hslb, *this));
  callback.add(new CDCDataFormatHandler(vname + ".dataformat", callback, hslb, *this));
  callback.add(new CDCWindowHandler(vname + ".window", callback, hslb, *this));
  callback.add(new CDCDelayHandler(vname + ".delay", callback, hslb, *this));
  callback.add(new CDCADCThresholdHandler(vname + ".adcth", callback, hslb, *this));
  callback.add(new CDCIndirectADCAccessHandler(vname + ".indirectadc", callback, hslb, *this));
  callback.add(new CDCDACControlHandler(vname + ".dac", callback, hslb, *this));
  callback.add(new CDCIndirectMonitorAccessHandler(vname + ".indirectmon", callback, hslb, *this));
  for (int i = 0; i < 48; i++) {
    std::string vname = StringUtil::form("cdc[%d].ped[%d]", hslb.get_finid(), i);
    callback.add(new CDCPedestalHandler(vname, callback, hslb, *this, i));
  }
}

void CDCFEE::boot(HSLB& hslb,  const FEEConfig& conf)
{
  const char* firmware = conf.getFirmware();
  if (firmware && File::exist(firmware)) {
    LogFile::info("Loading CDC FEE firmware: %s", firmware);
    std::string cmd = StringUtil::form("ssh ropc01 \"cd ~b2daq/run/cdc/; "
                                       "sh impact-batch.sh %s\"", firmware);
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
  const DBObject& obj(conf());
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
  val |= (obj.getInt("window") & 0xFF) << 8;
  val |= obj.getInt("delay") & 0xFF;
  hslb.writefee32(0x0012, val);

  // setting ADC threshold
  val = obj.getInt("adcth") & 0xFFFF;
  hslb.writefee32(0x0013, val);

  // setting DAC control
  val = obj.getInt("tdcth") & 0x7FFF;
  hslb.writefee32(0x0015, val);

  // setting Pedestals
  const DBObjectList o_ped(obj.getObjects("ped"));
  for (size_t i = 0; i < o_ped.size() / 2; i++) {
    val = o_ped[2 * i].getInt("val") & 0xFFFF
          | (o_ped[2 * i + 1].getInt("val") << 16) & 0xFFFF;
    hslb.writefee32(0x0020 + i * 2, val);
  }
}

extern "C" {
  void* getCDCFEE()
  {
    return new Belle2::CDCFEE();
  }
}
