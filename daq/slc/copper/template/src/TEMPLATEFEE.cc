#include "daq/slc/copper/template/TEMPLATEFEE.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

using namespace Belle2;

TEMPLATEFEE::TEMPLATEFEE()
{
}

void TEMPLATEFEE::init(RCCallback& callback, HSLB& hslb)
{

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

  // writing parameters to registers
  const DBObjectList objs(obj.getObjects("par"));
  for (DBObjectList::const_iterator it = objs.begin();
       it != objs.end(); it++) {
    const DBObject& o_par(*it);
    const std::string name = o_par.getText("name");
    int adr = o_par.getInt("adr");
    int val = o_par.getInt("val");
    LogFile::debug("writefee  val=%d to %s(adr=%d)", val, name.c_str(), adr);
    hslb.writefee32(adr, val);
  }
}

extern "C" {
  void* getTEMPLATEFEE()
  {
    return new Belle2::TEMPLATEFEE();
  }
}
