#include "daq/slc/copper/klm/KLMFEE.h"
#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#include <unistd.h>

using namespace Belle2;

KLMFEE::KLMFEE() : FEE("klm")
{
}

void KLMFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  std::string vname = StringUtil::form("klm[%d].", hslb.get_finid());
  callback.add(new FEE32Handler(vname + "test", callback, hslb, *this, 0x32));
}

void KLMFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
}

void KLMFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  // write access to register "reg0"
  LogFile::info("0x38>>0x%x", hslb.readfee32(0x38));
  hslb.writefee32(0x38, 10);
  int ret = hslb.readfee32(0x38);
  if (ret != 10) {
    LogFile::info("0x38>>%d", ret);
  } else {
    LogFile::warning("0x38>>%d!=10", ret);
  }
  int rcl = obj.getInt("rcl");
  hslb.writefee32(0x38, rcl);
  ret = hslb.readfee32(0x38);
  if (ret != rcl) {
    LogFile::info("0x38>>%d", ret);
  } else {
    LogFile::warning("0x38>>%d!=10", ret);
  }

  // writing stream file to FEE
  const std::string bitfile = obj.getText("bitfile");
  if (File::exist(bitfile)) {
    hslb.writestream(bitfile.c_str());
  } else {
    LogFile::error("file %s not exsits", bitfile.c_str());
  }
  sleep(2);

  // Write window values only to slot-a
  if (obj.hasObject("window")) {
    DBObject o_window(obj("window"));
    LogFile::debug("Write window values...");
    hslb.writefee32(0x30, o_window.getInt("val0"));
    hslb.writefee32(0x31, o_window.getInt("val1"));
    hslb.writefee32(0x32, o_window.getInt("val2"));
    hslb.writefee32(0x33, o_window.getInt("val3"));
  }
}

extern "C" {
  void* getKLMFEE()
  {
    return new Belle2::KLMFEE();
  }
}
