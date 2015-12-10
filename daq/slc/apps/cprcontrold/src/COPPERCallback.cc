#include "daq/slc/apps/cprcontrold/COPPERCallback.h"

#include "daq/slc/apps/cprcontrold/COPPERHandler.h"

#include <daq/slc/readout/ronode_status.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <mgt/libhslb.h>
#include <mgt/hsreg.h>

#include <cstring>

using namespace Belle2;

COPPERCallback::COPPERCallback(FEE* fee[4], bool dummymode)
{
  m_dummymode = dummymode;
  m_dummymode_org = dummymode;
  setTimeout(5);
  m_con.setCallback(this);
  for (int i = 0; i < 4; i++) {
    m_fee[i] = fee[i];
  }
  system("killall -9 basf2");
  m_force_boothslb = true;
}

COPPERCallback::~COPPERCallback() throw()
{
}

void COPPERCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  allocData(getNode().getName(), "ronode_status", ronode_status_revision);
  m_con.init("basf2", 1);
  if (!m_dummymode) {
    m_ttrx.open();
    m_copper.open();
  }
  m_flow.open(&m_con.getInfo());
  configure(obj);
}

void COPPERCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  try {
    m_dummymode = m_dummymode_org || (obj.hasValue("dummymode") && obj.getBool("dummymode"));
    add(new NSMVHandlerOutputPort(*this, "basf2.output.port"));
    add(new NSMVHandlerCOPPERROPID(*this, "basf2.pid"));
    add(new NSMVHandlerFifoEmpty(*this, "copper.err.fifoempty"));
    add(new NSMVHandlerFifoFull(*this, "copper.err.fifofull"));
    add(new NSMVHandlerLengthFifoFull(*this, "copper.err.lengthfifofull"));
    add(new NSMVHandlerTTRXBelle2LinkError(*this, "ttrx.err.b2linkerr"));
    const DBObject& o_ttrx(obj("ttrx"));
    add(new NSMVHandlerTTRXFirmware(*this, "ttrx.firm", 0,
                                    o_ttrx.hasText("firm") ? o_ttrx.getText("firm") : ""));
    for (int i = 0; i < 4; i++) {
      const DBObject& o_hslb(obj("hslb", i));
      if (m_dummymode || !m_fee[i] || !o_hslb.getBool("used")) continue;
      HSLB& hslb(m_hslb[i]);
      hslb.open(i);
      if (!(o_hslb.hasValue("dummyhslb") && o_hslb.getBool("dummyhslb"))) {
        m_fee[i]->init(*this, hslb);
      }
      std::string vname = StringUtil::form("hslb[%d]", i);
      add(new NSMVHandlerHSLBBelle2LinkDown(*this, vname + ".err.b2linkdown", i));
      add(new NSMVHandlerHSLBCOPPERFifoFull(*this, vname + ".err.cprfifofull", i));
      add(new NSMVHandlerHSLBCOPPERLengthFifoFull(*this, vname + ".err.cprlengthfifofull", i));
      add(new NSMVHandlerHSLBFifoFull(*this, vname + ".err.fifofull", i));
      add(new NSMVHandlerHSLBCRCError(*this, vname + ".err.crc", i));

      add(new NSMVHandlerHSLBFirmware(*this, vname + ".firm", i,
                                      o_hslb.hasText("firm") ? o_hslb.getText("firm") : ""));
      vname = StringUtil::form("hslb[%d]", i);
      add(new NSMVHandlerInt(vname + ".reg.adr", true, true, -1));
      add(new NSMVHandlerInt(vname + ".reg.size", true, true, -1));
      add(new NSMVHandlerHSLBRegValue(*this, vname + ".par.val", i));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".fmver", i, 0x81, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".b2lstat", i, 0x83, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".rxdata", i, 0x84, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".fwevt", i, 0x85, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".fwclk", i, 0x86, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".cntsec", i, 0x87, 4));
      if (m_fee[i] != NULL && obj.hasObject("fee")) {
        const DBObject& o_fee((obj("fee", i)));
        vname = StringUtil::form("fee[%d]", i);
        if (!(o_hslb.hasValue("dummyhslb") && o_hslb.getBool("dummyhslb"))) {
          add(new NSMVHandlerText(vname + ".name", true, false, m_fee[i]->getName()));
          vname = StringUtil::form("fee[%d]", i);
          if (o_fee.hasText("stream")) {
            add(new NSMVHandlerFEEStream(*this, vname + ".stream", i,
                                         o_fee.hasText("stream") ? o_fee.getText("stream") : ""));
          }
          add(new NSMVHandlerFEEBoot(*this, vname + ".boot", i));
          add(new NSMVHandlerFEELoad(*this, vname + ".load", i));
          vname = StringUtil::form("hslb[%d]", i);
          add(new NSMVHandlerHSLBTest(*this, vname + ".test", i));
          add(new NSMVHandlerHSLBCheckFee(*this, vname + ".checkfee", i));
          bool checked = hslb.checkfee() != "UNKNOWN";
          const hslb_info& info(hslb.getInfo());
          add(new NSMVHandlerInt(vname + ".hw", true, false, checked ? info.feehw : -1));
          add(new NSMVHandlerInt(vname + ".serial", true, false, checked ? info.feeserial : -1));
          add(new NSMVHandlerText(vname + ".type", true, false,
                                  checked ? HSLB::getFEEType(info.feehw) : "UNKNOWN"));
          add(new NSMVHandlerInt(vname + ".ver", true, false, checked ? info.feever : -1));
          add(new NSMVHandlerInt(vname + ".hslbid", true, false, checked ? info.hslbid : -1));
          add(new NSMVHandlerInt(vname + ".hslbver", true, false, checked ? info.hslbver : -1));
        } else {
          system((StringUtil::form("boothslb -%c ", (i + 'a')) + o_hslb.getText("firm")).c_str());
          system((StringUtil::form("/home/usr/b2daq/run/dumhslb/write-dumhslb -%c ",
                                   (i + 'a')) + o_fee.getText("randfile") +
                  " " + o_fee.getText("lengthfile")).c_str());
        }
      }
    }
  } catch (const std::exception& e) {
    throw (RCHandlerException(e.what()));
  }
}

void COPPERCallback::term() throw()
{
  m_con.abort();
  m_con.getInfo().unlink();
  for (int i = 0; i < 4; i++) {
    m_hslb[i].close();
  }
  m_copper.close();
  m_ttrx.close();
}

void COPPERCallback::boot(const DBObject& obj) throw(RCHandlerException)
{
  abort();
  if (!m_dummymode) {
    try {
      for (int i = 0; i < 4; i++) {
        const DBObject& o_hslb(obj("hslb", i));
        if (o_hslb.getBool("used") && m_fee[i] != NULL  && obj.hasObject("fee")) {
          const DBObject& o_fee(obj("fee", i));
          if (!(o_hslb.hasValue("dummyhslb") && o_hslb.getBool("dummyhslb"))) {
            HSLB& hslb(m_hslb[i]);
            hslb.open(i);
            FEE& fee(*m_fee[i]);
            try {
              fee.boot(hslb, o_fee);
            } catch (const IOException& e) {
              throw (RCHandlerException(e.what()));
            }
          } else {
          }
        }
      }
    } catch (const std::out_of_range& e) {
      throw (RCHandlerException(e.what()));
    }
  }
}

void COPPERCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  //m_dummymode = obj.hasValue("dummymode") && obj.getBool("dummymode");
  if (!m_dummymode) {
    m_ttrx.open();
    m_ttrx.monitor();
    if (m_ttrx.isError()) {
      m_ttrx.close();
      throw (RCHandlerException("TTRX Link error"));
    }
    try {
      for (int i = 0; i < 4; i++) {
        const DBObject& o_hslb(obj("hslb", i));
        if (o_hslb.getBool("used") && m_fee[i] != NULL) {
          HSLB& hslb(m_hslb[i]);
          hslb.open(i);
          if (!obj.hasObject("fee")) continue;
          if (!(o_hslb.hasValue("dummyhslb") && o_hslb.getBool("dummyhslb"))) {
            try {
              hslb.test();
              hslb.load();
            } catch (const HSLBHandlerException& e) {
              throw (RCHandlerException("Failed to load: %s", e.what()));
            }
            FEE& fee(*m_fee[i]);
            try {
              fee.load(hslb, (obj("fee", i)));
            } catch (const IOException& e) {
              throw (RCHandlerException(e.what()));
            }
          }
        }
      }
    } catch (const std::out_of_range& e) {
      throw (RCHandlerException(e.what()));
    }
  }
  bootBasf2(obj);
}

void COPPERCallback::start(int expno, int runno) throw(RCHandlerException)
{
  ronode_status* status = (ronode_status*)getData().get();
  status->stime = Time().getSecond();
  if (!m_con.start(expno, runno)) {
    throw (RCHandlerException("Failed to start"));
  }
}

void COPPERCallback::stop() throw(RCHandlerException)
{
  ronode_status* status = (ronode_status*)getData().get();
  status->stime = 0;
  m_con.stop();
}

bool COPPERCallback::pause() throw(RCHandlerException)
{
  LogFile::debug("Pausing");
  m_con.pause();
  while (m_con.getInfo().isRunning() || m_con.getInfo().isPausing()) {
    try {
      perform(wait(NSMNode(), RCCommand::ABORT, 1));
      LogFile::notice("Pause was canceled");
      return false;
    } catch (const TimeoutException& e) {
      LogFile::warning("Pasuing not finished yet");
    }
  }
  if (m_con.getInfo().isPaused()) {
    LogFile::info("Paused");
  } else {
    LogFile::warning("Pasuing was ignored");
  }
  return true;
}

bool COPPERCallback::resume(int /*subno*/) throw(RCHandlerException)
{
  LogFile::debug("Resuming");
  m_con.resume();
  while (m_con.getInfo().isPaused() || m_con.getInfo().isResuming()) {
    try {
      perform(wait(NSMNode(), RCCommand::ABORT, 1));
      LogFile::notice("Resume was canceled");
      return false;
    } catch (const TimeoutException& e) {
      LogFile::warning("Resuming not finished yet");
    }
  }
  if (m_con.getInfo().isRunning()) {
    LogFile::info("Resumed");
  } else {
    LogFile::warning("Resume was ignored");
  }
  return true;
}

void COPPERCallback::recover(const DBObject& obj) throw(RCHandlerException)
{
  //abort();
  load(obj);
}

void COPPERCallback::abort() throw(RCHandlerException)
{
  m_con.abort();
}

void COPPERCallback::logging(bool err, LogFile::Priority pri,
                             const char* str, ...) throw()
{
  if (err) {
    va_list ap;
    static char ss[1024];
    va_start(ap, str);
    vsprintf(ss, str, ap);
    va_end(ap);
    LogFile::put(pri, ss);
    reply(NSMMessage(DAQLogMessage(getNode().getName(), pri, ss)));
    if (pri >= LogFile::ERROR)
      setState(RCState::NOTREADY_S);
  }
}

void COPPERCallback::monitor() throw(RCHandlerException)
{
  if (!m_dummymode) {
    try {
      RCState state = getNode().getState();
      try {
        m_ttrx.monitor();
      } catch (const IOException& e) {
        return;
      }
      try {
        m_copper.monitor();
      } catch (const IOException& e) {
        return;
      }
      int dummymode = 0;
      get("dummymode", dummymode);
      if (!dummymode && state != RCState::NOTREADY_S && state.isStable()) {
        logging(m_copper.isFifoEmpty(), LogFile::NOTICE, "COPPER FIFO empty");
        logging(m_copper.isFifoFull(), LogFile::WARNING, "COPPER FIFO full");
        logging(m_copper.isLengthFifoFull(), LogFile::WARNING, "COPPER length FIFO full");
        for (int i = 0; i < 4; i++) {
          if (!m_fee[i]) continue;
          int used = 0;
          get(StringUtil::form("hslb[%d].used", i), used);
          if (used) {
            HSLB& hslb(m_hslb[i]);
            hslb.monitor();
            logging(hslb.isBelle2LinkDown(), LogFile::ERROR,
                    "HSLB %c Belle2 link down", (char)(i + 'a'));
            logging(hslb.isCOPPERFifoFull(), LogFile::WARNING,
                    "HSLB %c COPPER fifo full", (char)(i + 'a'));
            logging(hslb.isCOPPERLengthFifoFull(), LogFile::WARNING,
                    "HSLB %c COPPER length fifo full", (char)(i + 'a'));
            logging(hslb.isFifoFull(), LogFile::WARNING, "HSLB %c fifo full", (char)(i + 'a'));
            logging(hslb.isCRCError(), LogFile::WARNING, "HSLB %c CRC error", (char)(i + 'a'));
          }
        }
        logging(m_ttrx.isBelle2LinkError(), LogFile::ERROR, "TTRX Belle2 link error");
        logging(m_ttrx.isLinkUpError(), LogFile::ERROR, "TTRX Link up error");
      }
      NSMData& data(getData());
      if (data.isAvailable()) {
        ronode_status* nsm = (ronode_status*)data.get();
        if (m_flow.isAvailable()) {
          ronode_status& status(m_flow.monitor());
          memcpy(nsm, &status, sizeof(ronode_status));
        }
        double loads[3];
        if (getloadavg(loads, 3) > 0) {
          nsm->loadavg = (float)loads[0];
        } else {
          nsm->loadavg = -1;
        }
        data.flush();
      }
    } catch (const std::exception& e) {

    }
  }
  if (getNode().getState() == RCState::RUNNING_S ||
      getNode().getState() == RCState::READY_S ||
      getNode().getState() == RCState::PAUSED_S) {
    if (!m_con.isAlive()) {
      throw (RCHandlerException("Process down : basf2"));
    }
  }
}

void COPPERCallback::bootBasf2(const DBObject& obj) throw(RCHandlerException)
{
  if (m_con.isAlive()) return;
  std::string script;
  try {
    int flag = 0;
    for (size_t i = 0; i < 4; i++) {
      const DBObject& o_hslb(obj.getObject("hslb", i));
      if (m_fee[i] != NULL && o_hslb.getBool("used")) flag += 1 << i;
    }
    m_con.clearArguments();
    m_con.setExecutable("basf2");
    script = obj.getObject("basf2").getText("script");
    m_con.addArgument("%s/%s", getenv("BELLE2_LOCAL_DIR"),
                      script.c_str());
    m_con.addArgument(obj.getText("hostname"));
    std::string copperid_s = obj.getText("copperid");
    int id = atoi(copperid_s.substr(3).c_str());
    int copperid = id % 1000;//atoi(copperid_s.substr(4).c_str());
    int detectorid = id / 1000;
    char str[64];
    sprintf(str, "0x%d000000", detectorid);
    copperid += strtol(str, NULL, 0);
    m_con.addArgument(StringUtil::form("%d", copperid));
    m_con.addArgument(StringUtil::form("%d", flag));
    m_con.addArgument("1");
    const std::string nodename = StringUtil::tolower(getNode().getName());
    m_con.addArgument(nodename + "_" + "basf2");
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException(e.what()));
  }
  try {
    m_con.load(30);
  } catch (const std::exception& e) {
    LogFile::warning("load timeout");
  }
  LogFile::debug("load succeded");
}

bool COPPERCallback::isError() throw()
{
  if (m_copper.isError()) return true;
  for (int i = 0; i < 4; i++) {
    int used = 0;
    get(StringUtil::form("hslb[%d].used", i), used);
    if (used && m_hslb[i].isError()) return true;
  }
  if (m_ttrx.isBelle2LinkError())
    return true;
  return false;
}
