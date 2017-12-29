#include "daq/slc/copper/template/TEMPLATEFEE.h"
#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#define REG0_ADDR 0x01
#define REG1_ADDR 0x02

using namespace Belle2;

TEMPLATEFEE::TEMPLATEFEE()
{
}

void TEMPLATEFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  std::string vname = StringUtil::form("template[%d].", hslb.get_finid());
  callback.add(new FEE8Handler(vname + "reg0", callback, hslb, *this, REG0_ADDR));
  callback.add(new FEE8Handler(vname + "reg1", callback, hslb, *this, REG1_ADDR));
}

void TEMPLATEFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{

}

void TEMPLATEFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  // writing stream file to FEE
  const std::string stream = obj.getText("stream");
  if (File::exist(stream)) {
    hslb.writestream(stream.c_str());
  } else {
    LogFile::warning("file %s not exsits", stream.c_str());
  }

  // write access to register "reg0"
  int val = obj.getInt("reg0");
  LogFile::debug("writefee8 val=%d to reg0(adr=%d)", val, REG0_ADDR);
  hslb.writefee8(REG0_ADDR, val);

  // write access to register "reg1"
  val = obj.getInt("reg1");
  LogFile::debug("writefee8 val=%d to reg1(adr=%d)", val, REG1_ADDR);
  hslb.writefee8(REG1_ADDR, val);

}

extern "C" {
  void* getTEMPLATEFEE()
  {
    return new Belle2::TEMPLATEFEE();
  }
}
