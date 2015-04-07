#include "daq/slc/copper/ecl/ECLFEE.h"

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

void ECLFEE::boot(HSLB& hslb,  const DBObject&)
{
  hslb.writefn(0x30, 0);
  for (int ntry = 0; hslb.checkfee() == "UNKNOWN"; ntry++) {
    hslb.writefn32(0x82, 0x1000);
    usleep(100);
    hslb.writefn32(0x82, 0x10);
    if (ntry > 100) {
      throw (IOException("Can not establish b2link at HSLB %c",
                         (char)('a' + hslb.get_finid())));
    }
    usleep(100);
  }
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
}

extern "C" {
  void* getECLFEE()
  {
    return new Belle2::ECLFEE();
  }
}
