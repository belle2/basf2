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
#include <cstdlib>
#include <cstdio>
#include <sstream>

bool initialized = false;

using namespace Belle2;

std::string popen(const std::string& cmd)
{
  char buf[1000];
  FILE* fp;
  if ((fp = ::popen(cmd.c_str(), "r")) == NULL) {
    perror("can not exec commad");
    exit(EXIT_FAILURE);
  }
  std::stringstream ss;
  while (!feof(fp)) {
    memset(buf, 0, 1000);
    fgets(buf, sizeof(buf), fp);
    ss << buf << std::endl;
  }
  pclose(fp);
  return ss.str();
}

COPPERCallback::COPPERCallback(FEE* fee[4], bool dummymode)
{
  m_dummymode = dummymode;
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
  allocData(getNode().getName(), "ronode", ronode_status_revision);
  m_con.init("basf2", 1);
  if (!m_dummymode) {
    m_ttrx.open();
    m_copper.open();
  }
  m_flow.open(&m_con.getInfo());
  initialized = false;
  configure(obj);
  initialized = true;
  const std::string path_shm = "/cpr_pause_resume";
  if (!m_memory.open(path_shm, sizeof(int))) {
    perror("shm_open");
    LogFile::error("Failed to open %s", path_shm.c_str());
  }
  char* buf = (char*)m_memory.map(0, sizeof(int));
  memset(buf, 0, sizeof(int));
}

void COPPERCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  try {
    LogFile::info(obj.getName());
    add(new NSMVHandlerOutputPort(*this, "basf2.output.port"));
    add(new NSMVHandlerFifoEmpty(*this, "copper.err.fifoempty"));
    add(new NSMVHandlerFifoFull(*this, "copper.err.fifofull"));
    add(new NSMVHandlerLengthFifoFull(*this, "copper.err.lengthfifofull"));
    add(new NSMVHandlerInt("ttrx.err.b2link", true, false, 0));
    add(new NSMVHandlerInt("ttrx.err.linkup", true, false, 0));
    const DBObject& o_ttrx(obj("ttrx"));
    add(new NSMVHandlerTTRXFirmware(*this, "ttrx.firm", 0,
                                    o_ttrx.hasText("firm") ? o_ttrx.getText("firm") : ""));
    add(new NSMVHandlerFEELoadAll(*this, "loadfee"));
    //bool bootedttrx = false;
    for (int i = 0; i < 4; i++) {
      const DBObject& o_hslb(obj("hslb", i));
      if (m_dummymode || !m_fee[i] || !o_hslb.getBool("used")) continue;
      HSLB& hslb(m_hslb[i]);
      hslb.open(i);
      m_fee[i]->init(*this, hslb);
      std::string vname = StringUtil::form("hslb[%d]", i);
      add(new NSMVHandlerInt(vname + ".err.fifoempty", true, false, 0));
      add(new NSMVHandlerInt(vname + ".err.b2linkdown", true, false, 0));
      add(new NSMVHandlerInt(vname + ".err.cprfifofull", true, false, 0));
      add(new NSMVHandlerInt(vname + ".err.cprlengthfifofull", true, false, 0));
      add(new NSMVHandlerInt(vname + ".err.fifofull", true, false, 0));
      add(new NSMVHandlerInt(vname + ".err.crc", true, false, 0));
      add(new NSMVHandlerHSLBFirmware(*this, vname + ".firm", i,
                                      o_hslb.hasText("firm") ? o_hslb.getText("firm") : ""));
      add(new NSMVHandlerHSLBBoot(*this, vname + ".boot", i, "off"));
      vname = StringUtil::form("hslb[%d]", i);
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".fmver", i, 0x81, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".b2lstat", i, 0x83, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".rxdata", i, 0x84, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".fwevt", i, 0x85, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".fwclk", i, 0x86, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".cntsec", i, 0x87, 4));
      if (m_fee[i] != NULL && obj.hasObject("fee")) {
        const DBObject& o_fee((obj("fee", i)));
        vname = StringUtil::form("fee[%d]", i);
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
        //const hslb_info& info(hslb.getInfo());
      }
    }
  } catch (const std::exception& e) {
    initialized = true;
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

bool COPPERCallback::feeload()
{
  try {
    DBObject& obj(getDBObject());
    get(obj);
    for (int i = 0; i < 4; i++) {
      const DBObject& o_hslb(obj("hslb", i));
      if (o_hslb.getBool("used") && m_fee[i] != NULL) {
        HSLB& hslb(m_hslb[i]);
        hslb.open(i);
        if (!obj.hasObject("fee")) continue;
        try {
          hslb.test();
        } catch (const HSLBHandlerException& e) {
          throw (RCHandlerException("tesths failed : %s", e.what()));
        }
        FEE& fee(*m_fee[i]);
        try {
          fee.load(hslb, (obj("fee", i)));
        } catch (const IOException& e) {
          throw (RCHandlerException(e.what()));
        }
      }
    }
    return true;
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException(e.what()));
  }
  return false;
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
          try {
            hslb.test();
          } catch (const HSLBHandlerException& e) {
            throw (RCHandlerException("tesths failed : %s", e.what()));
          }
          FEE& fee(*m_fee[i]);
          try {
            fee.load(hslb, (obj("fee", i)));
          } catch (const IOException& e) {
            throw (RCHandlerException(e.what()));
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
  if (!m_con.start(expno, runno)) {
    throw (RCHandlerException("Failed to start"));
  }
}

void COPPERCallback::stop() throw(RCHandlerException)
{
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

void COPPERCallback::recover(const DBObject& /*obj*/) throw(RCHandlerException)
{
  //abort();
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
    log(pri, ss);
    if (pri >= LogFile::ERROR)
      setState(RCState::NOTREADY_S);
  }
}

void COPPERCallback::monitor() throw(RCHandlerException)
{
  RCState state = getNode().getState();
  try {
    int dummymode = 0;
    get("dummymode", dummymode);
    if (!dummymode) {
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
      if (state == RCState::RUNNING_S && state.isStable()) {
        logging(m_copper.isFifoFull(), LogFile::WARNING, "COPPER FIFO full");
        logging(m_copper.isLengthFifoFull(), LogFile::WARNING, "COPPER length FIFO full");
        set("copper.err.fifoempty", m_copper.isFifoEmpty());
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
            FEE& fee(*m_fee[i]);
            fee.monitor(*this, hslb);
            std::string vname = StringUtil::form("hslb[%d]", i);
            set(vname + ".err.b2linkdown", hslb.isBelle2LinkDown());
            set(vname + ".err.cprfifofull", hslb.isCOPPERFifoFull());
            set(vname + ".err.cprlengthfifofull", hslb.isCOPPERLengthFifoFull());
            set(vname + ".err.fifofull", hslb.isFifoFull());
            set(vname + ".err.crc", hslb.isCRCError());
          }
        }
        logging(m_ttrx.isBelle2LinkError(), LogFile::WARNING, "TTRX Belle2 link error");
        logging(m_ttrx.isLinkUpError(), LogFile::WARNING, "TTRX Link up error");
        set("ttrx.err.b2link", m_ttrx.isBelle2LinkError());
        set("ttrx.err.linkup", m_ttrx.isLinkUpError());
      }
      NSMData& data(getData());
      if (data.isAvailable()) {
        ronode_status* nsm = (ronode_status*)data.get();
        if (m_flow.isAvailable()) {
          ronode_status& status(m_flow.monitor());
          memcpy(nsm, &status, sizeof(ronode_status));
        } else {
          memset(nsm, 0, sizeof(ronode_status));
        }
        double loads[3];
        if (getloadavg(loads, 3) > 0) {
          nsm->loadavg = (float)loads[0];
        } else {
          nsm->loadavg = -1;
        }
        data.flush();
      }
    }
  } catch (const std::exception& e) {

  }
  if (getNode().getState() == RCState::RUNNING_S) {
    if (!m_con.isAlive()) {
      setState(RCState::ERROR_ES);
      log(LogFile::ERROR, "basf2 was down");
    }
  }
}

void COPPERCallback::bootBasf2(const DBObject& obj) throw(RCHandlerException)
{
  if (m_con.isAlive()) return;
  std::string script;
  try {
    int flag = 0;
    int nhslb = 0;
    for (size_t i = 0; i < 4; i++) {
      const DBObject& o_hslb(obj.getObject("hslb", i));
      if (m_fee[i] != NULL && o_hslb.getBool("used")) {
        flag += 1 << i;
        nhslb++;
      }
    }
    m_con.clearArguments();
    if (!m_dummymode) {
      m_con.setExecutable(StringUtil::form("%s/daq/ropc/des_ser_COPPER_main", getenv("BELLE2_LOCAL_DIR")));
      m_con.addArgument(obj.getText("hostname"));
      std::string copperid_s = obj.getText("copperid");
      int id = atoi(copperid_s.substr(3).c_str());
      int copperid = id % 1000;
      int detectorid = id / 1000;
      char str[64];
      sprintf(str, "0x%d000000", detectorid);
      copperid += strtol(str, NULL, 0);
      m_con.addArgument(StringUtil::form("%d", copperid));
      m_con.addArgument("%d", nhslb);
      m_con.addArgument("1");
      const std::string nodename = StringUtil::tolower(getNode().getName());
      m_con.addArgument(nodename + "_" + "basf2");
      try {
        m_con.load(30);
      } catch (const std::exception& e) {
        LogFile::warning("load timeout");
      }
    } else {
      m_con.setExecutable(StringUtil::form("%s/daq/rawdata/dummy_data_src", getenv("BELLE2_LOCAL_DIR")));
      std::string copperid_s = obj.getText("copperid");
      int id = atoi(copperid_s.substr(3).c_str());
      int copperid = id % 1000;
      int detectorid = id / 1000;
      char str[64];
      sprintf(str, "0x%d000000", detectorid);
      copperid += strtol(str, NULL, 0);
      m_con.addArgument(StringUtil::form("%d", copperid));
      m_con.addArgument("0");
      m_con.addArgument("%d", (obj.hasValue("nwords") ? obj.getInt("nwords") : 10));
      m_con.addArgument("1");
      m_con.addArgument("%d", flag);
      m_con.load(0);
    }
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException(e.what()));
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

