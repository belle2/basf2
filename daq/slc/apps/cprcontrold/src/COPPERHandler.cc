#include "daq/slc/apps/cprcontrold/COPPERHandler.h"

#include "daq/slc/apps/cprcontrold/COPPERCallback.h"

#include "daq/slc/copper/HSLB.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

bool NSMVHandlerOutputPort::handleGetInt(int& val)
{
  val = (int)m_callback.getProcess().getInfo().getOutputPort();
  return true;
}

bool NSMVHandlerCOPPERROPID::handleGetInt(int& val)
{
  val = (int)m_callback.getProcess().getProcess().get_id();
  return true;
}

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
    LogFile::info("Loading TTRX firmware: " + firmware);
    std::string cmd = "bootrx " + firmware;
    LogFile::info("programing ttrx : " + firmware);
    m_callback.set("ttrx.msg", "programing");
    system(cmd.c_str());
    m_callback.set("ttrx.msg", "program done");
    m_callback.log(LogFile::INFO, "program ttrx done");
    try {
      int used = 0;
      int val = 0;
      for (int i = 0; i < 4; i++) {
        std::string vname = StringUtil::form("hslb[%d].used", i);
        m_callback.get(vname, used);
        val += (used << i);
      }
      m_callback.getTTRX().write(0x130, val);
    } catch (const std::exception& e) {
      LogFile::error(e.what());
    }
    return true;
    /*
    bool ret = true;
    if ((ret = m_callback.getTTRX().boot(firmware))) {
      LogFile::info("Succeded");
    } else {
      LogFile::error("Failed");
    }
    return ret;
    */
  } else {
    LogFile::error("TTRX firmware %s not exists", firmware.c_str());
  }
  return false;
}

bool NSMVHandlerHSLBFirmware::handleSetText(const std::string& firmware)
{
  if (File::exist(firmware)) {
    LogFile::info("Loading HSLB firmware: " + firmware);
    try {
      if (m_hslb >= 0) {
        std::string cmd = StringUtil::form("booths -%c ", ('a' + m_hslb)) + firmware;
        LogFile::info("programing hslb-%c : %s", ('a' + m_hslb), firmware.c_str());
        m_callback.set(StringUtil::form("hslb[%d].msg", m_hslb), "programing");
        system(cmd.c_str());
        m_callback.set(StringUtil::form("hslb[%d].msg", m_hslb), "program done");
        m_callback.log(LogFile::INFO, "program hslb-%c done", ('a' + m_hslb));
        std::string emsg = "";
        for (int i = 0; i < 10; i++) {
          try {
            m_callback.log(LogFile::INFO, "test hslb-%c", m_hslb + 'a');
            m_callback.getHSLB(m_hslb).test();
            m_callback.log(LogFile::INFO, "test hslb-%c done", m_hslb + 'a');
            m_callback.set(StringUtil::form("hslb[%d].msg", m_hslb), "tesths done");
            return true;
          } catch (const HSLBHandlerException& e) {
            emsg = e.what();
          }
          m_callback.set(StringUtil::form("hslb[%d].msg", m_hslb), "tesths failed");
        }
        m_callback.log(LogFile::ERROR, "test hslb-%c failed %s", m_hslb + 'a', emsg.c_str());
      } else {
        for (int hslb = 0; hslb < 4; hslb++) {
          std::string cmd = StringUtil::form("booths -%c ", ('a' + hslb)) + firmware;
          LogFile::info("programing hslb-%c : %s", ('a' + hslb), firmware.c_str());
          m_callback.set("hslb.msg", "programing");
          system(cmd.c_str());
          m_callback.set("hslb.msg", "program done");
          m_callback.log(LogFile::INFO, "program hslb-%c done", ('a' + hslb));
          std::string emsg = "";
          bool success = false;
          for (int i = 0; i < 10; i++) {
            try {
              m_callback.log(LogFile::INFO, "test hslb-%c", hslb + 'a');
              m_callback.getHSLB(hslb).test();
              m_callback.log(LogFile::INFO, "test hslb-%c done", hslb + 'a');
              m_callback.set("hslb.msg", "tesths done");
              success = true;
              break;
            } catch (const HSLBHandlerException& e) {
              emsg = e.what();
            }
            if (!success) {
              m_callback.set("hslb.msg", "tesths failed");
              m_callback.log(LogFile::ERROR, "test hslb-%c failed %s", hslb + 'a', emsg.c_str());
            }
          }
        }
      }
      /*
        m_callback.getHSLB(m_hslb).bootfpga(firmware);
        LogFile::info("Succeded");
      return true;
      */
    } catch (const HSLBHandlerException& e) {
      LogFile::error("Failed : %s", e.what());
    }
  } else {
    LogFile::error("HSLB firmware %s not exists", firmware.c_str());
  }
  return false;
}

bool NSMVHandlerHSLBBoot::handleSetText(const std::string& val)
{
  std::string firmware = val;
  if (File::exist(firmware)) {
    LogFile::info("Loading HSLB firmware: " + firmware);
    try {
      std::string cmd = StringUtil::form("booths -%c ", ('a' + m_hslb)) + firmware;
      system(cmd.c_str());
      //m_callback.getHSLB(m_hslb).bootfpga(firmware);
      cmd = StringUtil::form("tesths -%c ", ('a' + m_hslb));
      system(cmd.c_str());
      m_callback.log(LogFile::INFO, "Loeded HSLB firm " + firmware);
      return true;
    } catch (const HSLBHandlerException& e) {
      m_callback.log(LogFile::ERROR, "Failed to load HSLB firm " + firmware);
    }
  } else {
    m_callback.log(LogFile::ERROR, "HSLB firmware %s not exists",
                   firmware.c_str());
  }
  return false;
}

bool NSMVHandlerFEEStream::handleSetText(const std::string& stream)
{
  if (File::exist(stream)) {
    LogFile::info("Streaming file: " + stream);
    try {
      m_callback.getHSLB(m_hslb).writestream(stream.c_str());
      LogFile::info("Succeded");
      return true;
    } catch (const HSLBHandlerException& e) {
      LogFile::error("Failed : %s", e.what());
    }
  } else {
    LogFile::error("FEE stream file %s not exists", stream.c_str());
  }
  return false;
}

bool NSMVHandlerFEEBoot::handleSetText(const std::string& val)
{
  DBObject& obj(m_callback.getDBObject());
  m_callback.get(obj);
  if (val == "on" && m_callback.getFEE(m_hslb)) {
    FEE& fee(*m_callback.getFEE(m_hslb));
    HSLB& hslb(m_callback.getHSLB(m_hslb));
    try {
      fee.boot(m_callback, hslb, m_callback.getFEEDB(m_hslb));
      return true;
    } catch (const IOException& e) {
      LogFile::error(e.what());
    }
  }
  return false;
}

bool NSMVHandlerFEELoad::handleSetText(const std::string& val)
{
  DBObject& obj(m_callback.getDBObject());
  m_callback.get(obj);
  LogFile::debug("load fee");
  if (val == "on" && m_callback.getFEE(m_hslb)) {
    FEE& fee(*m_callback.getFEE(m_hslb));
    HSLB& hslb(m_callback.getHSLB(m_hslb));
    try {
      fee.load(m_callback, hslb, m_callback.getFEEDB(m_hslb));
      return true;
    } catch (const IOException& e) {
      LogFile::error(e.what());
    }
  }
  return false;
}

bool NSMVHandlerFEELoadAll::handleSetInt(int val)
{
  if (val > 0) {
    return m_callback.feeload();
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
        val = m_callback.getHSLB(m_hslb).readfee32(m_adr);
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
      HSLB& hslb();
      if (m_size == 1) {
        m_callback.getHSLB(m_hslb).writefn(m_adr, val);
      } else if (m_size == 4) {
        m_callback.getHSLB(m_hslb).writefn32(m_adr, val);
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
        val = m_callback.getHSLB(m_hslb).readfn(m_adr);
      } else if (m_size == 4) {
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

bool NSMVHandlerHSLBTest::handleGetText(std::string& val)
{
  if (val.size() > 0) {
    for (int i = 0; i < 10; i++) {
      try {
        m_callback.log(LogFile::INFO, "testing hslb:%c", m_hslb + 'a');
        m_callback.set(StringUtil::form("hslb[%d].msg", m_hslb), "tesths");
        val = m_callback.getHSLB(m_hslb).test();
        m_callback.log(LogFile::INFO, "test done hslb:%c", m_hslb + 'a');
        m_callback.set(StringUtil::form("hslb[%d].msg", m_hslb), "tesths done");
        return true;
      } catch (const HSLBHandlerException& e) {
        m_callback.log(LogFile::ERROR, "test failed hslb:%c %s", m_hslb + 'a', e.what());
      }
    }
    m_callback.set(StringUtil::form("hslb[%d].msg", m_hslb), "tesths failed");
  }
  return false;
}

bool NSMVHandlerHSLBUsed::handleSetInt(int val)
{
  NSMVHandlerInt::handleSetInt(val);
  try {
    int used = 0;
    val = 0;
    for (int i = 0; i < 4; i++) {
      std::string vname = StringUtil::form("hslb[%d].used", i);
      m_callback.get(vname, used);
      val += (used << i);
    }
    m_callback.getTTRX().write(0x130, val);
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool NSMVHandlerHSLBUsed::handleGetInt(int& val)
{
  return NSMVHandlerInt::handleGetInt(val);
}

/*
bool NSMVHandlerHSLBCheckFee::handleGetText(std::string& val)
{
  LogFile::info("check FEE (HSLB:%c)", m_hslb + 'a');
  HSLB& hslb(m_callback.getHSLB(m_hslb));
  if ((val = hslb.checkfee()) != "UNKNOWN") {
    std::string vname = StringUtil::split(m_name, '.')[0];
    const hslb_info& info(hslb.getInfo());
    m_callback.set(vname + ".hw", info.feehw);
    m_callback.set(vname + ".serial", info.feeserial);
    m_callback.set(vname + ".type", HSLB::getFEEType(info.feehw));
    m_callback.set(vname + ".ver", info.feever);
    m_callback.set(vname + ".hslbid", info.hslbid);
    m_callback.set(vname + ".hslbver", info.hslbver);
    LogFile::info(StringUtil::replace(val.c_str(), "\n", ", "));
  } else {
    LogFile::error("check FEE error");
  }
  return true;
}
*/
