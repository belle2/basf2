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

// registers that won't return values -- don't check
unsigned int ReadIgnoreRegs[] = { 0x2800, 0x2C00, 0x3000, 0x3400,   // IRSX direct. carrier 0
                                  0x4800, 0x4C00, 0x5000, 0x5400,   // IRSX direct, carrier 1
                                  0x6800, 0x6C00, 0x7000, 0x7400,   // IRSX direct, carrier 2
                                  0x8800, 0x8C00, 0x9000, 0x9400,   // IRSX direct, carrier 3
                                  0x0206, 0x0207, 0x04AA, 0x04B0,   // Misc. SCROD write-only
                                  0x04FF,                           //   "
                                  0x0600, 0x0800, 0x0A00, 0x0C00,   // Misc. PGP registers
                                  0x0601, 0x0801, 0x0A01, 0x0C01,   //   "
                                  0x0602, 0x0802, 0x0A02, 0x0C02,   //   "
                                  0x2206, 0x4206, 0x6206, 0x8206,   // Misc. carrier write-only
                                  0x2207, 0x4207, 0x6207, 0x8207,   //   "
                                  0x24AA, 0x44AA, 0x64AA, 0x84AA,   //   "
                                  0x24AB, 0x44AB, 0x64AB, 0x84AB,   //   "
                                  0x24B0, 0x44B0, 0x64B0, 0x84B0,   //   "
                                  0x24FF, 0x44FF, 0x64FF, 0x84FF
                                };   //   "

TOPFEE::TOPFEE()
{
  LogFile::debug("%s:%d", __FILE__, __LINE__);
}

void TOPFEE::init(RCCallback& callback, HSLB& hslb)
{
  LogFile::debug("%s:%d", __FILE__, __LINE__);
  TOPDAQ::add_scrod(hslb, callback);
  LogFile::debug("%s:%d", __FILE__, __LINE__);
  for (int carrier = 0; carrier < 4; carrier++) {
    TOPDAQ::add_carrier(hslb, callback, carrier);
  }
  LogFile::debug("%s:%d", __FILE__, __LINE__);
  monitor(callback, hslb);
  LogFile::debug("%s:%d", __FILE__, __LINE__);
}

void TOPFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{

}

void TOPFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  hslb.writefn32(HSREGL_RESET, 0xffffff);
  TOPDAQ::config1boardstack(hslb, callback);

  // write access to register "reg0"
  //if (obj.hasValue("reg0")) {
  //  int val = obj.getInt("reg0");
  //  LogFile::debug("writefee8 val=%d to reg0(adr=%d)", val, REG0_ADDR);
  //  hslb.writefee8(REG0_ADDR, val);
  //}
}

void TOPFEE::monitor(RCCallback& callback, HSLB& hslb)
{
  LogFile::debug("%s:%d", __FILE__, __LINE__);
  TOPDAQ::get_status(hslb, callback);
  LogFile::debug("%s:%d", __FILE__, __LINE__);
}

extern "C" {
  void* getTOPFEE()
  {
    return new Belle2::TOPFEE();
  }
}

