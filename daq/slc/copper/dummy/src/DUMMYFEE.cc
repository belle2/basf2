#include "daq/slc/copper/dummy/DUMMYFEE.h"
#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

using namespace Belle2;

DUMMYFEE::DUMMYFEE() : FEE("dummy")
{
}

void DUMMYFEE::init(RCCallback& callback, HSLB& hslb,  const DBObject& obj)
{

}

void DUMMYFEE::boot(RCCallback& callback, HSLB& hslb,  const DBObject& obj)
{

}

void DUMMYFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{

}

extern "C" {
  void* getDUMMYFEE()
  {
    return new Belle2::DUMMYFEE();
  }
}
