#include "daq/slc/copper/trg/TRGFEE.h"
#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#define REG0_ADDR 0x01
#define REG1_ADDR 0x02

using namespace Belle2;

TRGFEE::TRGFEE() : FEE("trg")
{
}

void TRGFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
}

void TRGFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
}

void TRGFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
}

extern "C" {
  void* getTRGFEE()
  {
    return new Belle2::TRGFEE();
  }
}
