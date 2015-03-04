#include "daq/slc/apps/cprcontrold/COPPERHandler.h"

#include "daq/slc/apps/cprcontrold/COPPERCallback.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

using namespace Belle2;

bool NSMVHandlerFifoEmpty::handleGetInt(int& val)
{
  val = (int)m_callback.getCopper().isFifoEmpty();
  return true;
}

bool NSMVHandlerFifoFull::handleGetInt(int& val)
{
  val = (int)m_callback.getCopper().isFifoFull();
  return true;
}

bool NSMVHandlerLengthFifoFull::handleGetInt(int& val)
{
  val = (int)m_callback.getCopper().isLengthFifoFull();
  return true;
}

bool NSMVHandlerHSLBBelle2LinkDown::handleGetInt(int& val)
{
  val = (int)m_callback.getHSLB(m_hslb).isBelle2LinkDown();
  return true;
}

bool NSMVHandlerHSLBCOPPERFifoFull::handleGetInt(int& val)
{
  val = (int)m_callback.getHSLB(m_hslb).isCOPPERFifoFull();
  return true;
}

bool NSMVHandlerHSLBCOPPERLengthFifoFull::handleGetInt(int& val)
{
  val = (int)m_callback.getHSLB(m_hslb).isCOPPERLengthFifoFull();
  return true;
}

bool NSMVHandlerHSLBFifoFull::handleGetInt(int& val)
{
  val = (int)m_callback.getHSLB(m_hslb).isFifoFull();
  return true;
}

bool NSMVHandlerHSLBCRCError::handleGetInt(int& val)
{
  val = (int)m_callback.getHSLB(m_hslb).isCRCError();
  return true;
}

bool NSMVHandlerTTRXBelle2LinkError::handleGetInt(int& val)
{
  val = (int)m_callback.getTTRX().isBelle2LinkError();
  return true;
}

bool NSMVHandlerTTRXLinkUpError::handleGetInt(int& val)
{
  val = (int)m_callback.getTTRX().isLinkUpError();
  return true;
}

bool NSMVHandlerDownloadTTRXFirmware::handleSetInt(int val)
{
  if (val > 0) {
    LogFile::debug("booting ttrx");
    std::string firmware;
    m_callback.get("setup.ttrx_firmware", firmware);
    if (File::exist(firmware)) {
      return m_callback.getTTRX().boot(firmware);
    } else {
      LogFile::error("TTRX firmware %s not exists", firmware.c_str());
    }
  }
  return true;
}

bool NSMVHandlerDownloadHSLBFirmware::handleSetInt(int val)
{
  if (val > 0) {
    LogFile::debug("booting hslb:%c", (m_hslb + 'a'));
    std::string firmware;
    m_callback.get("setup.hslb_firmware", firmware);
    if (File::exist(firmware)) {
      return m_callback.getHSLB(m_hslb).bootfpga(firmware);
    } else {
      LogFile::error("HSLB firmware %s not exists", firmware.c_str());
    }
  }
  return true;
}

bool NSMVHandlerHSLBRegValue::handleSetInt(int val)
{
  try {
    if (m_adr == 0) {
      m_callback.get(StringUtil::replace(getName(), ".reg", ".adr"), m_adr);
    }
    if (m_adr > 0) {
      LogFile::debug("wrting %d-th HSLB : %d >> %s=%d", m_hslb, val, getName().c_str(), m_adr);
      return true;
    }
  } catch (const std::exception& e) {
  }
  return false;
}

bool NSMVHandlerHSLBRegValue::handleGetInt(int& val)
{
  try {
    if (m_adr == 0) {
      m_callback.get(StringUtil::replace(getName(), ".reg", ".adr"), m_adr);
    }
    if (m_adr > 0) {
      val = rand() % 256;
      LogFile::debug("reading %d-th HSLB : %d << %s=%d", m_hslb, val, getName().c_str(), m_adr);
      return true;
    }
  } catch (const std::exception& e) {
  }
  return false;
}
