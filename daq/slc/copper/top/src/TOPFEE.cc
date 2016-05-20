#include "daq/slc/copper/top/TOPFEE.h"
#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#define REG0_ADDR 0x01

using namespace Belle2;

TOPFEE::TOPFEE()
{
}

void TOPFEE::init(RCCallback& callback, HSLB& hslb)
{
  std::string vname = StringUtil::form("top[%d].", hslb.get_finid());
  callback.add(new FEEStreamHandler(vname + "steam", callback, hslb, ""));
  callback.add(new FEE8Handler(vname + "reg0", callback, hslb, *this, REG0_ADDR));
}

void TOPFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{

}

void TOPFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{

  // writing stream file to FEE
  if (obj.hasText("stream")) {
    const std::string stream = obj.getText("stream");
    if (File::exist(stream)) {
      //hslb.writestream(stream.c_str());
    } else {
      LogFile::warning("file %s not exsits", stream.c_str());
    }
  }

  // write access to register "reg0"
  if (obj.hasValue("reg0")) {
    //  int val = obj.getInt("reg0");
    //  LogFile::debug("writefee8 val=%d to reg0(adr=%d)", val, REG0_ADDR);
    //  hslb.writefee8(REG0_ADDR, val);
  }
}

extern "C" {
  void* getTOPFEE()
  {
    return new Belle2::TOPFEE();
  }
}
