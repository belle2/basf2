#include "daq/slc/copper/cdc/CDCFEE.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <cstdlib>

#include <mgt/hsreg.h>

using namespace Belle2;

CDCFEE::CDCFEE()
{
}

void CDCFEE::init(RCCallback& callback)
{

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
    val = 1 << 24;
  } else if (mode == "raw") {
    LogFile::info("raw mode");
    val = 2 << 24;
  } else if (mode == "raw_suppress") {
    LogFile::info("raw-suppress mode");
    val = 3 << 24;
  } else {
    LogFile::warning("no readout");
    val = 0;
  }
  val |= (obj.getInt("window") & 0xF) << 8;
  val |= obj.getInt("delay") & 0xF;
  hslb.writefee32(0x0012, val);

  // setting ADC threshold
  val = obj.getInt("adcth") & 0xFF;
  hslb.writefee32(0x0013, val);

  // setting Pedestals
  const DBObjectList o_ped(obj.getObjects("ped"));
  for (size_t i = 0; i < o_ped.size() / 2; i++) {
    val = o_ped[2 * i].getInt("val") & 0xF;
    val = (o_ped[2 * i + 1].getInt("val") << 16 & 0xF);
    hslb.writefee32(0x0020 + i * 2, val);
  }
}

extern "C" {
  void* getCDCFEE()
  {
    return new Belle2::CDCFEE();
  }
}
