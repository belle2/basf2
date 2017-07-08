#include "daq/slc/apps/cprcontrold/COPPERHandler.h"

#include "daq/slc/apps/cprcontrold/COPPERCallback.h"

#include "daq/slc/copper/HSLB.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

bool NSMVHandlerTTRXBoot::handleSetText(const std::string& firmware)
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
  } else {
    LogFile::error("TTRX firmware %s not exists", firmware.c_str());
  }
  return false;
}

bool NSMVHandlerHSLBBoot::handleSetText(const std::string& firmware)
{
  if (File::exist(firmware)) {
    LogFile::info("Loading HSLB firmware: " + firmware);
    try {
      for (int i = 0; i < 4; i++) {
        if (m_hslb >= 0 && i != m_hslb) continue;
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
            HSLB& hslb(m_callback.getHSLB(m_hslb));
            m_callback.tesths(hslb);
            m_callback.staths(hslb);
            m_callback.log(LogFile::INFO, "test hslb-%c done", m_hslb + 'a');
            m_callback.set(StringUtil::form("hslb[%d].msg", m_hslb), "tesths done");
            return true;
          } catch (const HSLBHandlerException& e) {
            emsg = e.what();
          }
          m_callback.set(StringUtil::form("hslb[%d].msg", m_hslb), "tesths failed");
        }
        m_callback.log(LogFile::ERROR, "test hslb-%c failed %s", m_hslb + 'a', emsg.c_str());
      }
    } catch (const HSLBHandlerException& e) {
      LogFile::error("Failed : %s", e.what());
    }
  } else {
    LogFile::error("HSLB firmware %s not exists", firmware.c_str());
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

bool NSMVHandlerHSLBTest::handleGetText(std::string& val)
{
  for (int i = 0; i < 10; i++) {
    try {
      m_callback.log(LogFile::INFO, "testing hslb:%c", m_hslb + 'a');
      m_callback.set(StringUtil::form("hslb[%d].msg", m_hslb), "tesths");
      HSLB& hslb(m_callback.getHSLB(m_hslb));
      val = m_callback.tesths(hslb);
      m_callback.staths(hslb);
      m_callback.log(LogFile::INFO, "test done hslb:%c", m_hslb + 'a');
      m_callback.set(StringUtil::form("hslb[%d].msg", m_hslb), "tesths done");
      return true;
    } catch (const HSLBHandlerException& e) {
      m_callback.log(LogFile::ERROR, "test failed hslb:%c %s", m_hslb + 'a', e.what());
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

