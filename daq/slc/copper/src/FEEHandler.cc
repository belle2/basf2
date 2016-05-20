#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/copper/FEE.h>
#include <daq/slc/copper/HSLB.h>

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include "mgt/hsreg.h"

#include <unistd.h>

using namespace Belle2;

bool FEE8Handler::feeget(int& val)
{
  try {
    val = m_hslb.readfee8(m_adr);
    if (m_bitmax > 0) {
      val = (bitmask(m_bitmax, m_bitmin) & val) >> m_bitmin;
    }
    LogFile::info("FEE read8 0x%x >> %x", m_adr, val);
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
    LogFile::info("FEE write8 0x%x << %x", m_adr, val);
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
    if (m_bitmax > 0) {
      val = (bitmask(m_bitmax, m_bitmin) & val) >> m_bitmin;
    }
    LogFile::info("FEE read32 0x%x >> %x", m_adr, val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
    return false;
  }
  return true;
}

bool FEE32Handler::feeset(int val)
{
  try {
    if (m_bitmax > 0) {
      int mask = bitmask(m_bitmax, m_bitmin);
      val = (val << m_bitmin) & mask;
      val |= m_hslb.readfee32(m_adr) & (~mask);
    }
    m_hslb.writefee32(m_adr, val);
    LogFile::info("FEE write32 0x%x << %x", m_adr, val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
    return false;
  }
  return true;
}

unsigned int FEEHandler::bitmask(unsigned int max, unsigned int min)
{
  unsigned int bit = 0;
  for (unsigned int i = min; i <= max; i++) {
    bit |= 0x1 << i;
  }
  return bit;
}

bool FEEStreamHandler::handleSetText(const std::string& val)
{
  NSMVHandlerText::handleSetText(val);
  const std::string stream = val;
  if (File::exist(stream)) {
    m_hslb.writestream(stream.c_str());
  } else {
    LogFile::warning("file %s not exsits", stream.c_str());
    return false;
  }
  return true;
}
