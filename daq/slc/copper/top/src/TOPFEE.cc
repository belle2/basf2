#include "daq/slc/copper/top/TOPFEE.h"
#include "daq/slc/copper/top/b2l_top.h"
#include "daq/slc/copper/top/b2l_top_asic.h"
#include "daq/slc/copper/top/b2l_top_board.h"
#include "daq/slc/copper/top/log_temps.h"
#include "daq/slc/copper/top/topreg.h"

#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#include <unistd.h>

using namespace Belle2;

TOPFEE::TOPFEE()
{
}

void TOPFEE::init(RCCallback& callback, HSLB& hslb)
{
  TOPDAQ::add_scrod(hslb, callback);
  for (int carrier = 0; carrier < 4; carrier++) {
    TOPDAQ::add_carrier(hslb, callback, carrier);
  }
  monitor(callback, hslb);
}

void TOPFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  hslb.writefn32(HSREGL_RESET, 0xffffff);
  TOPDAQ::config1boardstack(hslb, callback);
}

void TOPFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  // write access to register "reg0"
  //if (obj.hasValue("reg0")) {
  //  int val = obj.getInt("reg0");
  //  LogFile::debug("writefee8 val=%d to reg0(adr=%d)", val, REG0_ADDR);
  //  hslb.writefee8(REG0_ADDR, val);
  //}
}

void TOPFEE::monitor(RCCallback& callback, HSLB& hslb)
{
  TOPDAQ::get_status(hslb, callback);
}

extern "C" {
  void* getTOPFEE()
  {
    return new Belle2::TOPFEE();
  }
}

