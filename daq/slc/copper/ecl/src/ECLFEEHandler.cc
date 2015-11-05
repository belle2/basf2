#include "daq/slc/copper/ecl/ECLFEEHandler.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/copper/FEE.h>
#include <daq/slc/copper/HSLB.h>

#include "mgt/hsreg.h"

#include <unistd.h>

using namespace Belle2;

bool ECLFEEHandler::feeget(int& val)
{
  try {
    val = m_hslb.readfee8(m_adr);
    LogFile::info("ECL FEE read 0x%x >> %d", m_adr, val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool ECLFEEHandler::feeset(int val)
{
  try {
    m_hslb.writefee8(m_adr, val);
    LogFile::info("ECL FEE write 0x%x << %d", m_adr, val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;

}
