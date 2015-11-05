#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/copper/FEE.h>
#include <daq/slc/copper/HSLB.h>

#include "mgt/hsreg.h"

#include <unistd.h>

using namespace Belle2;

bool FEE8Handler::feeget(int& val)
{
  try {
    val = m_hslb.readfee8(m_adr);
    LogFile::info("FEE read8 0x%x >> %d", m_adr, val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
    return false;
  }
  return true;
}

bool FEE8Handler::feeset(int val)
{
  try {
    m_hslb.writefee8(m_adr, val);
    LogFile::info("FEE write8 0x%x << %d", m_adr, val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
    return false;
  }
  return true;
}

bool FEE32Handler::feeget(int& val)
{
  try {
    val = m_hslb.readfee32(m_adr);
    LogFile::info("FEE read32 0x%x >> %d", m_adr, val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
    return false;
  }
  return true;
}

bool FEE32Handler::feeset(int val)
{
  try {
    m_hslb.writefee32(m_adr, val);
    LogFile::info("FEE write32 0x%x << %d", m_adr, val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
    return false;
  }
  return true;
}
