#include "daq/slc/copper/template/TEMPLATEFEE.h"
#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

using namespace Belle2;

TEMPLATEFEE::TEMPLATEFEE()
{
}

void TEMPLATEFEE::init(RCCallback& callback, HSLB& hslb)
{
  std::string vname = StringUtil::form("template[%d].", hslb.get_finid());
  callback.add(new FEE8Handler(vname + "reg0", callback, hslb, *this, 0x01));
  callback.add(new FEE8Handler(vname + "reg1", callback, hslb, *this, 0x02));
}

void TEMPLATEFEE::boot(HSLB& hslb,  const DBObject& obj)
{

}

void TEMPLATEFEE::load(HSLB& hslb, const DBObject& obj)
{
  // writing stream file to FEE
  const std::string stream = obj.getText("stream");
  if (File::exist(stream)) {
    hslb.writestream(stream.c_str());
  } else {
    LogFile::warning("file %s not exsits", stream.c_str());
  }

  // write access to register "reg0"
  int adr = obj("reg0").getInt("adr");
  int val = obj("reg0").getInt("val");
  LogFile::debug("writefee8 val=%d to reg0(adr=%d)", val, adr);
  hslb.writefee8(adr, val);

  // write access to register "reg1"
  adr = obj("reg1").getInt("adr");
  val = obj("reg1").getInt("val");
  LogFile::debug("writefee8 val=%d to reg1(adr=%d)", val, adr);
  hslb.writefee8(adr, val);

}

extern "C" {
  void* getTEMPLATEFEE()
  {
    return new Belle2::TEMPLATEFEE();
  }
}
