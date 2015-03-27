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

bool NSMVHandlerTTRXFirmware::handleSetText(const std::string& firmware)
{
  if (File::exist(firmware)) {
    return m_callback.getTTRX().boot(firmware);
  } else {
    LogFile::error("TTRX firmware %s not exists", firmware.c_str());
  }
  return false;
}

bool NSMVHandlerHSLBFirmware::handleSetText(const std::string& firmware)
{
  if (File::exist(firmware)) {
    return m_callback.getHSLB(m_hslb).bootfpga(firmware);
  } else {
    LogFile::error("HSLB firmware %s not exists", firmware.c_str());
  }
  return false;
}

bool NSMVHandlerFEEStream::handleSetText(const std::string& stream)
{
  if (File::exist(stream)) {
    return m_callback.getHSLB(m_hslb).writestream(stream.c_str());
  } else {
    LogFile::error("FEE stream file %s not exists", stream.c_str());
  }
  return false;
}

bool NSMVHandlerFEEBoot::handleSetInt(int val)
{
  FEEConfig fconf;
  if (val > 0 && m_callback.getFEE(m_hslb) &&
      fconf.read(m_callback.getDBObject())) {
    FEE& fee(*m_callback.getFEE(m_hslb));
    HSLB& hslb(m_callback.getHSLB(m_hslb));
    try {
      fee.boot(hslb, fconf);
      return true;
    } catch (const IOException& e) {
      LogFile::error(e.what());
    }
  }
  return false;
}

bool NSMVHandlerHSLBRegValue::handleSetInt(int val)
{
  try {
    std::string vname = StringUtil::replace(getName(), ".par.val", ".reg.adr");
    m_callback.get(vname, m_adr);
    vname = StringUtil::replace(getName(), "par.val", "reg.size");
    m_callback.get(vname, m_size);
    if (m_adr > 0) {
      if (m_size == 1) {
        m_callback.getHSLB(m_hslb).writefee8(m_adr, val);
      } else if (m_size == 4) {
        m_callback.getHSLB(m_hslb).writefee32(m_adr, val);
      }
      LogFile::info("wrting HSLB-%c : %d to (adr=%d, size=%d)", ('a' + m_hslb), val, m_adr, m_size);
      return true;
    }
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
  return false;
}

bool NSMVHandlerHSLBRegValue::handleGetInt(int& val)
{
  try {
    std::string vname = StringUtil::replace(getName(), ".par.val", ".reg.adr");
    m_callback.get(vname, m_adr);
    vname = StringUtil::replace(getName(), "par.val", "reg.size");
    m_callback.get(vname, m_size);
    if (m_adr >= 0) {
      val = 0;
      if (m_size == 1) {
        val = m_callback.getHSLB(m_hslb).readfee8(m_adr);
      } else if (m_size == 4) {
        m_callback.getHSLB(m_hslb).readfee32(m_adr, &val);
      } else if (m_adr < 0x80) {
        val = m_callback.getHSLB(m_hslb).readfn(m_adr);
      } else if (m_size > 0) {
        val = m_callback.getHSLB(m_hslb).readfn32(m_adr);
      }
      LogFile::info("reading HSLB-%c : %d from (adr=%d, size=%d)", ('a' + m_hslb), val, m_adr, m_size);
      return true;
    }
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
  return false;
}

bool NSMVHandlerHSLBRegFixed::handleSetInt(int val)
{
  try {
    if (m_adr > 0) {
      if (m_size == 1) {
        m_callback.getHSLB(m_hslb).writefee8(m_adr, val);
      } else if (m_size == 4) {
        m_callback.getHSLB(m_hslb).writefee32(m_adr, val);
      }
      LogFile::info("wrting HSLB-%c : %d to (adr=%d, size=%d)", ('a' + m_hslb), val, m_adr, m_size);
      return true;
    }
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
  return false;
}

bool NSMVHandlerHSLBRegFixed::handleGetInt(int& val)
{
  try {
    if (m_adr >= 0) {
      val = 0;
      if (m_size == 1) {
        val = m_callback.getHSLB(m_hslb).readfee8(m_adr);
      } else if (m_size == 4) {
        m_callback.getHSLB(m_hslb).readfee32(m_adr, &val);
      } else if (m_adr < 0x80) {
        val = m_callback.getHSLB(m_hslb).readfn(m_adr);
      } else {
        val = m_callback.getHSLB(m_hslb).readfn32(m_adr);
      }
      LogFile::info("reading HSLB-%c : %d from (adr=%d, size=%d)", ('a' + m_hslb), val, m_adr, m_size);
      return true;
    }
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
  return false;
}

bool NSMVHandlerHSLBLinkFee::handleSetInt(int val)
{
  if (val > 0) {
    LogFile::info("link FEE (HSLB:%c)", m_hslb + 'a');
    return m_callback.getHSLB(m_hslb).linkfee();
  } else {
    LogFile::error("Failed to link FEE (HSLB:%c)", m_hslb + 'a');
  }
  return false;
}

bool NSMVHandlerHSLBUnLinkFee::handleSetInt(int val)
{
  if (val > 0) {
    LogFile::info("unlink FEE (HSLB:%c)", m_hslb + 'a');
    return m_callback.getHSLB(m_hslb).linkfee();
  } else {
    LogFile::error("Failed to unlink FEE (HSLB:%c)", m_hslb + 'a');
  }
  return false;
}

bool NSMVHandlerHSLBTrgOnFee::handleSetInt(int val)
{
  if (val > 0) {
    LogFile::info("trgon FEE (HSLB:%c)", m_hslb + 'a');
    return m_callback.getHSLB(m_hslb).trgonfee();
  } else {
    LogFile::error("Failed to trig on FEE (HSLB:%c)", m_hslb + 'a');
  }
  return false;
}

bool NSMVHandlerHSLBTrgOffFee::handleSetInt(int val)
{
  if (val > 0) {
    LogFile::info("trgoff FEE (HSLB:%c)", m_hslb + 'a');
    return m_callback.getHSLB(m_hslb).trgofffee();
  } else {
    LogFile::error("Failed to trg off FEE (HSLB:%c)", m_hslb + 'a');
  }
  return false;
}

bool NSMVHandlerHSLBCheckFee::handleGetText(std::string& val)
{
  LogFile::info("check FEE (HSLB:%c)", m_hslb + 'a');
  val = m_callback.getHSLB(m_hslb).checkfee();
  return true;
}

