#include "daq/slc/copper/arich/ARICHFEE.h"
#include "daq/slc/copper/arich/N6SA0xHSLB.h"

#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#include <sstream>

#define ADR_REG0 0x0010
#define ADR_REG1 0x0011
#define ADR_REG2 0x0012
#define ADR_REG3 0x0013
#define MASK_REV 31,24
#define MASK_FEECNT0 31,24

using namespace Belle2;

ARICHFEE::ARICHFEE()
{
}

void ARICHFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  std::string vname = StringUtil::form("arich[%d].", hslb.get_finid());
  //callback.add(new FEE32Handler(vname + "syn_date", callback, hslb, *this, ADR_REG0));
  //callback.add(new FEE32Handler(vname + "rev", callback, hslb, *this, ADR_REG1, MASK_REV));
}

void ARICHFEE::boot(RCCallback& callback, HSLB& hslb,  const DBObject& obj)
{

}

void ARICHFEE::write_read(HSLB& hslb, int adr, int val) throw(HSLBHandlerException)
{
  hslb.writefee32(adr, val);
  LogFile::debug("writefee32 val=%d to adr=%d", val, adr);
  int ret = hslb.readfee32(adr);
  if (ret != val) {
    LogFile::error("failed to write to adr %x (w:%x, r:%x)", adr, val, ret);
    throw (HSLBHandlerException("failed to write to adr %x (w:%x, r:%x)", adr, val, ret));
  }
}

void logmask(const std::string& name, int mask, int max)
{
  std::stringstream ss;
  ss << name << StringUtil::form(": %x", mask);
  for (int i = 0; i < max; i++) {
    ss << i << ":" << ((mask >> i) & 0x1) << " ";
  }
  LogFile::debug(ss.str().c_str());
}

void ARICHFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  //m_fee.load(hslb, obj);
}

extern "C" {
  void* getARICHFEE()
  {
    return new Belle2::ARICHFEE();
  }
}
