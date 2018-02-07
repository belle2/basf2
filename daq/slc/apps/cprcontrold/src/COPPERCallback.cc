#include "daq/slc/apps/cprcontrold/COPPERCallback.h"

#include "daq/slc/apps/cprcontrold/COPPERHandler.h"

#include <daq/slc/readout/ronode_status.h>

#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/File.h>
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

#define CRCERR     0x68
#define B2LCSR     0x69
#define HSD32      0x6e
#define HSA32      0x6f
#define HSLBHW     0x70
#define HSLBFW     0x71
#define HSLSTAT    0x72
#define HSLCONT    0x73
#define CCLK       0x74
#define CONF       0x75
#define FEEHW      0x76
#define FEESERIAL  0x77
#define FEEVER     0x79
#define FEETYPE    0x79
#define SERIAL_L   0x7b
#define SERIAL_H   0x7c
#define TYPE_L     0x7d
#define TYPE_H     0x7e
#define HSLBID     0x80
#define HSLBVER    0x81
#define RESET      0x82
#define B2LSTAT    0x83
#define RXDATA     0x84
#define FWEVT      0x85
#define FWCLK      0x86
#define CNTSEC     0x87
#define NWORD      0x88
#define ERRDET     0x8c
#define ERRPAT1    0x8d
#define ERRPAT2    0x8e
#define B2LCTIM    0x91
#define B2LTAG     0x92
#define B2LUTIM    0x93
#define B2LERUN    0x94

#ifndef D
#define D(a,b,c) (((a)>>(c))&((1<<((b)+1-(c)))-1))
#define B(a,b)   D(a,b,b)
#define Bs(a,b,s)   (B(a,b)?(s):"")
#define Ds(a,b,c,s)   (D(a,b,c)?(s):"")
#endif

bool initialized = false;

namespace Belle2 {
  class FEELoad {
  public:
    FEELoad(COPPERCallback& callback, FEE& fee, HSLB& hslb, const DBObject& obj)
      : m_callback(callback), m_fee(fee), m_hslb(hslb), m_obj(obj) {}
  public:
    void run()
    {
      m_fee.load(m_callback, m_hslb, m_obj);
    }
  private:
    COPPERCallback& m_callback;
    FEE& m_fee;
    HSLB& m_hslb;
    const DBObject& m_obj;
  };
  class FEEBoot {
  public:
    FEEBoot(COPPERCallback& callback, FEE& fee, HSLB& hslb, const DBObject& obj)
      : m_callback(callback), m_fee(fee), m_hslb(hslb), m_obj(obj) {}
  public:
    void run()
    {
      m_fee.boot(m_callback, m_hslb, m_obj);
    }
  private:
    COPPERCallback& m_callback;
    FEE& m_fee;
    HSLB& m_hslb;
    const DBObject& m_obj;
  };
}

using namespace Belle2;

std::string popen(const std::string& cmd)
{
  char buf[1000];
  FILE* fp;
  if ((fp = ::popen(cmd.c_str(), "r")) == NULL) {
    perror("can not exec commad");
    return "";
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

COPPERCallback::COPPERCallback(FEE* fee[4])
{
  setTimeout(5);
  m_con.setCallback(this);
  for (int i = 0; i < 4; i++) {
    m_fee[i] = fee[i];
  }
  system("killall -9 des_ser_COPPER_main");
}

COPPERCallback::~COPPERCallback() throw()
{
}

void COPPERCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  allocData(getNode().getName(), "ronode", ronode_status_revision);
  m_con.init("basf2", 1);
  m_ttrx.open();
  m_copper.open();
  m_flow.open(&m_con.getInfo());
  initialized = false;
  configure(obj);
  initialized = true;
  const std::string path_shm = "/cpr_pause_resume";
  if (!m_memory.open(path_shm, sizeof(int))) {
    perror("shm_open");
  }
  char* buf = (char*)m_memory.map(0, sizeof(int));
  memset(buf, 0, sizeof(int));
}

void COPPERCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  try {
    add(new NSMVHandlerInt("copper.err.ffull", true, false, 0));
    add(new NSMVHandlerInt("copper.err.lffull", true, false, 0));
    add(new NSMVHandlerInt("ttrx.b2lerr", true, false, 0));
    add(new NSMVHandlerInt("ttrx.linkerr", true, false, 0));
    add(new NSMVHandlerText("ttrx.msg", true, false, ""));
    const DBObject& o_ttrx(obj("ttrx"));
    std::string rxfw = o_ttrx.hasText("firm") ? o_ttrx.getText("firm") : "";
    add(new NSMVHandlerTTRXBoot(*this, "ttrx.boot", 0, rxfw));
    for (int i = 0; i < 4; i++) {
      const DBObject& o_hslb(obj("hslb", i));
      std::string vname = StringUtil::form("hslb[%d].", i);
      add(new NSMVHandlerHSLBUsed(*this, vname + "used", i, m_fee[i] && o_hslb.getBool("used")));
      HSLB& hslb(m_hslb[i]);
      if (m_fee[i] && o_hslb.getBool("used")) {
        hslb.open(i);
        m_o_fee[i] = dbload(obj("fee", i).getText("path"));
        m_o_fee[i].print();
        m_fee[i]->init(*this, hslb, m_o_fee[i]);
      }
      std::string hsfw = o_hslb.hasText("firm") ? o_hslb.getText("firm") : "";
      add(new NSMVHandlerText("hslb.fw", true, true, hsfw));
      add(new NSMVHandlerText("hslb.msg", true, false, ""));
      add(new NSMVHandlerHSLBBoot(*this, vname + "boot", i, hsfw));
      add(new NSMVHandlerText(vname + "msg", true, false, ""));
      add(new NSMVHandlerInt(vname + "id", true, false, 0));
      add(new NSMVHandlerInt(vname + "ver", true, false, 0));
      add(new NSMVHandlerInt(vname + "invalid", true, false, 1));
      add(new NSMVHandlerText(vname + "feename", true, false, "unknown"));
      add(new NSMVHandlerInt(vname + "feeser", true, false, 0));
      add(new NSMVHandlerInt(vname + "feever", true, false, 0));
      add(new NSMVHandlerInt(vname + "b2ldown", true, false, 0));
      add(new NSMVHandlerInt(vname + "b2llost", true, false, 0));
      add(new NSMVHandlerInt(vname + "ffstate", true, false, 0));
      add(new NSMVHandlerInt(vname + "pll", true, false, 0));
      add(new NSMVHandlerInt(vname + "pll2", true, false, 0));
      add(new NSMVHandlerInt(vname + "masked", true, false, 0));
      add(new NSMVHandlerInt(vname + "fful", true, false, 0));
      add(new NSMVHandlerInt(vname + "nwff", true, false, 0));
      add(new NSMVHandlerInt(vname + "rxdata", true, false, 0));
      add(new NSMVHandlerInt(vname + "rxldown", true, false, 0));
      add(new NSMVHandlerInt(vname + "rxcrce", true, false, 0));
      add(new NSMVHandlerInt(vname + "feecrce", true, false, 0));
      add(new NSMVHandlerInt(vname + "nevent", true, false, 0));
      add(new NSMVHandlerInt(vname + "ktypes", true, false, 0));
      add(new NSMVHandlerInt(vname + "evtsz.avg", true, false, 0));
      add(new NSMVHandlerInt(vname + "evtsz.last", true, false, 0));
      add(new NSMVHandlerInt(vname + "evtzs.max", true, false, 0));
      add(new NSMVHandlerText(vname + "staths", true, false, ""));
      add(new NSMVHandlerInt(vname + "b2lerr", true, false, 0));
      if (m_fee[i] && o_hslb.getBool("used")) {
        //const DBObject& o_fee(m_o_fee[i]);
        //m_fee[i]->readback(*this, hslb, o_fee);
        add(new NSMVHandlerText(vname + "name", true, false, m_fee[i]->getName()));
        vname = StringUtil::form("fee[%d].", i);
        add(new NSMVHandlerFEEBoot(*this, vname + "boot", i, ""));
        add(new NSMVHandlerFEELoad(*this, vname + "load", i, ""));
        vname = StringUtil::form("hslb[%d]", i);
        add(new NSMVHandlerHSLBTest(*this, vname + "tesths", i));
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

void COPPERCallback::boot(const std::string& opt, const DBObject& obj) throw(RCHandlerException)
{
  std::string firmware;
  if (opt.find("ttrx") != std::string::npos) {
    const DBObject& o_ttrx(obj("ttrx"));
    firmware = o_ttrx.getText("firm");
    if (File::exist(firmware)) {
      set("ttrx.msg", "programing");
      log(LogFile::INFO, "bootrx %s", firmware.c_str());
      system(("bootrx " + firmware).c_str());
      set("ttrx.msg", "program done");
      log(LogFile::INFO, "bootrx done");
    }
  }
  int val = 0;
  for (int i = 0; i < 4; i++) {
    const DBObject& o_hslb(obj("hslb", i));
    std::string vname = StringUtil::form("hslb[%d]", i);
    add(new NSMVHandlerHSLBUsed(*this, vname + ".used", i, m_fee[i] && o_hslb.getBool("used")));
    if (m_fee[i] && o_hslb.getBool("used"))
      val += (1 << i);
  }
  for (int i = 0; i < 10; i++) {
    m_ttrx.write(0x130, val);
    int ret = (m_ttrx.read(0x130) & 0xF);
    if (val == ret) {
      break;
    }
    log(LogFile::WARNING, "ttrx-130 is not consistent : (%x>>%x)", val, ret);
    usleep(50000);
  }

  bool success[4] = {false, false, false, false};
  for (int i = 0; i < 4; i++) {
    const DBObject& o_hslb(obj("hslb", i));
    if (!m_fee[i] || !o_hslb.getBool("used")) continue;
    if (m_fee[i]->getName() == "dummy") continue;
    HSLB& hslb(m_hslb[i]);
    firmware = o_hslb.getText("firm");
    if (opt.find("hslb") != std::string::npos) {
      if (File::exist(firmware)) {
        log(LogFile::INFO, "booths -%c %s", ('a' + i), firmware.c_str());
        set("hslb.msg", "programing");
        std::string cmd = StringUtil::form("booths -%c ", ('a' + i)) + firmware;
        system(cmd.c_str());
        set("hslb.msg", "program done");
        log(LogFile::INFO, "booths -%c done", ('a' + i));
      }
    }
    std::string emsg = "";
    for (int j = 0; j < 3; j++) {
      try {
        log(LogFile::INFO, "test hslb-%c", i + 'a');
        tesths(hslb);
        log(LogFile::INFO, "test hslb-%c done", i + 'a');
        set("hslb.msg", "tesths done");
        success[i] = true;
        break;
      } catch (const HSLBHandlerException& e) {
        emsg = e.what();
      }
      if (!success[i]) {
        set("hslb.msg", "tesths failed");
        log(LogFile::ERROR, "test hslb-%c failed %s", i + 'a', emsg.c_str());
      }
    }
  }
  try {
    for (int i = 0; i < 4; i++) {
      if (!m_fee[i]) continue;
      if (m_fee[i]->getName() == "dummy") continue;
      const DBObject& o_hslb(obj("hslb", i));
      if (o_hslb.getBool("used") && obj.hasObject("fee")) {
        const DBObject& o_fee(m_o_fee[i]);
        HSLB& hslb(m_hslb[i]);
        hslb.open(i);
        if (success[i]) {
          FEE& fee(*m_fee[i]);
          try {
            PThread th(new FEEBoot(*this, fee, hslb, o_fee));
            while (th.is_alive()) {
              wait(1);
            }
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

void COPPERCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  bool done_tesths = true;
  m_ttrx.open();
  m_ttrx.monitor();
  if (m_ttrx.isError()) {
    m_ttrx.close();
    //throw (RCHandlerException("TTRX Link error"));
  }
  try {
    int flag = 0;
    int nhslb = 0;
    const DBObjectList& o_hslbs(obj.getObjects("hslb"));
    for (size_t i = 0; i < o_hslbs.size(); i++) {
      if (o_hslbs[i].getBool("used")) {
        flag += 1 << i;
        nhslb++;
      }
    }
    for (int i = 0; i < 10; i++) {
      m_ttrx.write(0x130, flag);
      int flag_ret = (m_ttrx.read(0x130) & 0xF);
      if (flag == flag_ret) {
        break;
      }
      log(LogFile::WARNING, "ttrx-130 is not consistent : (%x>>%x)", flag, flag_ret);
      usleep(50000);
    }
    for (size_t i = 0; i < o_hslbs.size(); i++) {
      if (!m_fee[i]) continue;
      const DBObject& o_hslb(obj("hslb", i));
      if (!o_hslb.getBool("used")) continue;
      HSLB& hslb(m_hslb[i]);
      hslb.open(i);
      if (!m_fee[i]) continue;
      if (m_fee[i]->getName() == "dummy") continue;
      if (!obj.hasObject("fee")) continue;
      std::string vname = StringUtil::form("hslb[%d]", i);
      int count = 0;
      std::string emsg;
      for (count = 0; count < 10; count++) {
        try {
          tesths(hslb);
          count = 0;
          break;
        } catch (const HSLBHandlerException& e) {
          count++;
          emsg = e.what();
          usleep(500);
        }
      }
      staths(hslb);
      FEE& fee(*m_fee[i]);
      try {
        const DBObject& o_fee(m_o_fee[i]);
        PThread th(new FEELoad(*this, fee, hslb, o_fee));
        while (th.is_alive()) {
          wait(1);
        }
      } catch (const std::exception& e) {
        LogFile::error("Error in HSLB-%c", (i + 'a'));
        throw (RCHandlerException(e.what()));
      }
    }
  } catch (const std::out_of_range& e) {
    LogFile::error("Error in HSLB");
    throw (RCHandlerException(e.what()));
  }
  if (!done_tesths) {
    throw (RCHandlerException("Failed to load"));
  }
  bootBasf2(obj);
}

void COPPERCallback::start(int expno, int runno) throw(RCHandlerException)
{
  for (int i = 0; i < 4; i++) {
    if (!m_fee[i]) continue;
    HSLB& hslb(m_hslb[i]);
    FEE& fee(*m_fee[i]);
    try {
      fee.start(*this, hslb);
    } catch (const IOException& e) {
      throw (RCHandlerException(e.what()));
    }
  }
  if (!m_con.start(expno, runno)) {
    throw (RCHandlerException("Failed to start"));
  }
}

void COPPERCallback::stop() throw(RCHandlerException)
{
  m_con.stop();
  for (int i = 0; i < 4; i++) {
    if (!m_fee[i]) continue;
    HSLB& hslb(m_hslb[i]);
    FEE& fee(*m_fee[i]);
    try {
      fee.stop(*this, hslb);
    } catch (const IOException& e) {
      throw (RCHandlerException(e.what()));
    }
  }
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
  abort();
  load(obj);
}

void COPPERCallback::abort() throw(RCHandlerException)
{
  stop();
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
      setState(RCState::ERROR_ES);
  }
}

void COPPERCallback::monitor() throw(RCHandlerException)
{
  RCState state = getNode().getState();
  try {
    try {
      m_ttrx.monitor();
    } catch (const IOException& e) {
      LogFile::warning(e.what());
      return;
    }
    try {
      m_copper.monitor();
    } catch (const IOException& e) {
      LogFile::warning(e.what());
      return;
    }
    set("copper.lffull", m_copper.isLengthFifoFull());
    set("copper.ffull", m_copper.isFifoFull());
    for (int i = 0; i < 4; i++) {
      if (!m_fee[i]) continue;
      if (m_fee[i]->getName() == "dummy") continue;
      int used = 0;
      get(StringUtil::form("hslb[%d].used", i), used);
      if (used) {
        HSLB& hslb(m_hslb[i]);
        staths(hslb);
        FEE& fee(*m_fee[i]);
        fee.monitor(*this, hslb);
      }
    }
    set("ttrx.b2lerr", m_ttrx.isBelle2LinkError());
    set("ttrx.linkerr", m_ttrx.isLinkUpError());
  } catch (const std::exception& e) {
    LogFile::warning(e.what());
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
  if (m_con.isAlive()) {
    m_con.abort();
  }
  try {
    int flag = 0;
    for (size_t i = 0; i < 4; i++) {
      if (!m_fee[i]) continue;
      const DBObject& o_hslb(obj.getObject("hslb", i));
      if (m_fee[i] != NULL && o_hslb.getBool("used")) {
        flag += 1 << i;
      }
    }
    m_con.clearArguments();
    m_con.setExecutable("/home/usr/b2daq/cprdaq/bin/des_ser_COPPER_main");
    m_con.addArgument(obj.getText("hostname"));
    std::string copperid_s = obj.getText("copperid");
    int id = atoi(copperid_s.substr(3).c_str());
    int copperid = id % 1000;
    int detectorid = id / 1000;
    char str[64];
    sprintf(str, "0x%d000000", detectorid);
    copperid += strtol(str, NULL, 0);
    m_con.addArgument(StringUtil::form("%d", copperid));
    m_con.addArgument("%d", flag);
    m_con.addArgument("1");
    const std::string nodename = StringUtil::tolower(getNode().getName());
    m_con.addArgument(nodename + "_" + "basf2");
    try {
      m_con.load(30);
    } catch (const std::exception& e) {
      LogFile::warning("load timeout");
    }
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException(e.what()));
  }
  LogFile::debug("load succeded");
}

int COPPERCallback::tesths(HSLB& hslb) throw (HSLBHandlerException)
{
  int i = hslb.get_finid();
  int tmpval = 0;
  int* valp = &tmpval;
  m_warning = 0;
  m_errcode = 0;

  int id;
  int ver;
  int csr;
  int j;

  id  = hslb.readfn32(HSREGL_ID);   /* 0x80 */
  ver = hslb.readfn32(HSREGL_VER);  /* 0x81 */
  csr = hslb.readfn32(HSREGL_STAT); /* 0x83 */

  if (id != 0x48534c42 && id != 0x48534c37) {
    *valp = id;
    m_errcode = EHSLB_NOTFOUND;
    throw (HSLBHandlerException("hslb-%c not found (id=0x%08x != 0x48534c42)", 'a' + i, id));
  }
  if (id == 0x48534c42 && ver < 34) {
    *valp = ver;
    m_errcode = EHSLB_TOOOLD;
    throw (HSLBHandlerException("hslb-%c too old firmware (ver=0.%02d < 0.34)", 'a' + i, ver));
  }
  if (id == 0x48534c37 && ver < 6) {
    *valp = ver;
    m_errcode = EHSLB7_TOOOLD;
    throw (HSLBHandlerException("hslb-%c too old firmware (ver=0.%02d < 0.06)", 'a' + i, ver));
  }
  if (csr & 2) {
    /*
      if it is just disabled, don't just use it, and if all are
      disabled, it makes use_slot == 0 and can be stopped.
      m_errcode = EHSLB_DISABLED; */
    m_errcode = EHSLB_DISABLED;
    throw (HSLBHandlerException("hslb-%c is disabled, ttrx reg 130 bit%d=0", 'a' + i, i));
  }
  if (csr & 0x20000000) {
    int j;
    int recvok = 0;
    int uptime0 = hslb.readfn32(HSREGL_UPTIME);
    int uptime1;
    usleep(1000 * 1000);
    uptime1 = hslb.readfn32(HSREGL_UPTIME);

    if (uptime0 == 0) {
      m_errcode = EHSLB_CLOCKNONE;
      throw (HSLBHandlerException("hslb-%c clock is missing", 'a' + i));
    } else if (uptime0 == uptime1) {
      m_errcode = EHSLB_CLOCKLOST;
      throw (HSLBHandlerException("hslb-%c clock is lost or too slow", 'a' + i));
    } else if (uptime1 > uptime0 + 1) {
      m_errcode = EHSLB_CLOCKFAST;
      throw (HSLBHandlerException("hslb-%c clock is too fast", 'a' + i));
    }

    for (j = 0; j < 100; j++) {
      int recv = hslb.readfn32(HSREGL_RXDATA) & 0xffff;
      if (recv == 0x00bc) recvok++;
    }
    if (recvok < 80) {
      m_errcode = EHSLB_PLLLOST;
      throw (HSLBHandlerException("hslb-%c PLL lost and can't receive data (csr=%08x)",
                                  'a' + i, csr));
    }
    m_warning = WHSLB_PLLLOST;
    log(LogFile::WARNING, "hslb-%c PLL lost (csr=%08x) is probably harmless and ignored",
        'a' + i, csr);
    csr &= ~0x20000000;
  }
  for (j = 0; j < 100; j++) {
    int csr2 = hslb.readfn32(HSREGL_STAT); /* 0x83 */
    if ((csr ^ csr2) & 0x100) break;
  }
  if (j == 100) csr |= 0x20000000;

  /*
    bit 00000001 - link is not established
    bit 00000002 - hslb is disabled
    bit 00000020 - bad 127MHz detected
    bit 00000040 - GTP PLL not locked (never happen?)
    bit 00000080 - PLL2 not locked
    bit 00000100 - LSB of statff, should be toggling
    bit 20000000 - PLL1 not locked, but somehow not correctly working,
    so it is reused for j=100 condition
    bit 80000000 - link down happened in the past
  */

  if ((csr & 0x200000e1)) {
    int count;
    int oldcsr = csr;

    count = hslb.reset_b2l(csr);

    if ((csr & 0x200000e1)) {
      *valp = csr;
      if (csr & 1) {
        m_errcode = EHSLB_B2LDOWN;
        throw (HSLBHandlerException("hslb-%c Belle2link is down, csr=%08x", 'a' + i, csr));
      } else if (csr & 0x20) {
        m_errcode = EHSLB_CLOCKBAD;
        throw (HSLBHandlerException("hslb-%c bad clock detected, csr=%08x", 'a' + i, csr));
      } else if (csr & 0x80) {
        m_errcode = EHSLB_PLL2LOST;
        throw (HSLBHandlerException("hslb-%c PLL2 lock lost, csr=%08x", 'a' + i, csr));
      } else if (csr & 0x40) {
        m_errcode = EHSLB_GTPPLL;
        throw (HSLBHandlerException("hslb-%c GTP PLL lock lost, csr=%08x", 'a' + i, csr));
      } else if (csr & 0x20000000) {
        m_errcode = EHSLB_FFCLOCK;
        throw (HSLBHandlerException("hslb-%c FF clock is stopped, csr=%08x", 'a' + i, csr));
      }
    } else {
      *valp = count;
      if (oldcsr & 1) {
        m_warning = WHSLB_B2LDOWN;
        log(LogFile::WARNING, "hslb-%c Belle2link recovered, csr=%08x (retry %d)",
            'a' + i, csr, count);
      } else if (oldcsr & 0x20) {
        m_warning = WHSLB_B2LDOWN;
        log(LogFile::WARNING, "hslb-%c bad clock recovered, csr=%08x (retry %d)",
            'a' + i, csr, count);
      } else if (oldcsr & 0x80) {
        m_warning = WHSLB_PLL2LOST;
        log(LogFile::WARNING, "hslb-%c PLL2 lock recovered, csr=%08x (retry %d)",
            'a' + i, csr, count);
      } else if (oldcsr & 0x40) {
        m_warning = WHSLB_GTPPLL;
        log(LogFile::WARNING, "hslb-%c GTP PLL lock recovered, csr=%08x (retry %d)",
            'a' + i, csr, count);
      } else if (oldcsr & 0x20000000) {
        m_warning = WHSLB_FFCLOCK;
        log(LogFile::WARNING, "hslb-%c FF clock is recovered, csr=%08x (retry %d)",
            'a' + i, csr, count);
      }
    }
  }

  /* 2015.0605.1524 this doesn't seem to be a solution when statepr != 0 */
  /* 2015.0618.1408 although not perfect, still better than nothing */
  if (csr & 0x000f0000) {
    hslb.readfee32(0);
    csr = hslb.readfn32(HSREGL_STAT); /* 0x83 */
  }

  if ((csr & 0x5fffeec1) != 0x18000000 && (csr & 0x200000e1) == 0) {
    m_errcode = EHSLB_BADSTATE;
    *valp = csr;
    throw (HSLBHandlerException("hslb-%c hslb in bad state (csr=%08x)", 'a' + i, csr));
  }
  //printf("hslb-%c 0.%02d %08x", 'a'+i, ver, csr);
  return 0;//m_errcode < 0 ? m_errcode : use_slot;
}

/* ---------------------------------------------------------------------- *\
   staths
\* ---------------------------------------------------------------------- */
std::string COPPERCallback::staths(HSLB& hslb) throw (HSLBHandlerException)
{
  hsreg_t h;
  double total;
  char prompt[128];
  char msg[1280];
  char ss[128];
  char state[128];

  hslb.hsreg_read(h);
  hslb.hsreg_getfee(h);

  int i = hslb.get_finid();
  std::string vname = StringUtil::form("hslb[%d].", i);
  set(vname + "id", (int)h.hslbid);
  if (h.hslbid != 0x48534c42) {
    if (h.fintyp != 0x000a) {
      sprintf(ss, "HSLB-%c invalid fintyp=%04x != 000a\n", 'a' + i, h.fintyp);
      strcat(msg, ss);
    } else if (!(h.conf & 0x80)) {
      sprintf(ss, "HSLB-%c firmware is not programmed\n", 'a' + i);
      strcat(msg, ss);
    } else {
      sprintf(ss, "HSLB-%c invalid firmware=%08x != 48534c42\n",
              'a' + i, h.hslbid);
      strcat(msg, ss);
    }
    set(vname + "ver", 0);
    set(vname + "invalid", 1);
    set(vname + "feename", "unknown");
    set(vname + "feeser", 0);
    set(vname + "feever", 0);
    set(vname + "b2ldown", 0);
    set(vname + "b2llost", 0);
    set(vname + "ffstate", 0);
    set(vname + "pll", 0);
    set(vname + "pll2", 0);
    set(vname + "masked", 0);
    set(vname + "fful", 0);
    set(vname + "nwff", 0);
    set(vname + "rxdata", 0);
    set(vname + "rxldown", 0);
    set(vname + "rxcrce", 0);
    set(vname + "feecrce", 0);
    set(vname + "nevent", 0);
    set(vname + "ktypes", 0);
    set(vname + "evtsz.avg", 0);
    set(vname + "evtsz.last", 0);
    set(vname + "evtzs.max", 0);
    set(vname + "staths", "");
    set(vname + "b2lerr", 0);
    logging((getNode().getState() == RCState::RUNNING_S ||
             getNode().getState() == RCState::LOADING_TS),
            LogFile::WARNING, "hslb-%c : %s", 'a' + i, ss);
    return "unknown";
  }

  sprintf(prompt, "(%c)    ", i + 'a');

  sprintf(ss, "HSLB-%c version %d.%02d / ",
          i + 'a', h.hslbver / 100, h.hslbver % 100);
  set(vname + "ver", (int)h.hslbver);
  strcat(msg, ss);
  if (h.hslbsta & 1) {
    sprintf(ss, "b2link is down\n");
    strcat(msg, ss);
    set(vname + "fee.name", "b2ldown");
    set(vname + "fee.ser", 0);
    set(vname + "fee.ver", 0);
  } else if (h.feeser & 0x8000) {
    sprintf(ss, "fee info is not available\n");
    strcat(msg, ss);
    set(vname + "fee.name", "No fee");
    set(vname + "fee.ser", 0);
    set(vname + "fee.ver", 0);
  } else {
    sprintf(ss, "%s serial %d version %d\n",
            feename(h.feehw, h.feefw), h.feeser, h.feever);
    strcat(msg, ss);
    set(vname + "fee.name", feename(h.feehw, h.feefw));
    set(vname + "fee.ser", h.feeser);
    set(vname + "fee.ver", h.feever);
  }
  if (h.hslbver < 46) {
    sprintf(ss, "%s", prompt);
    strcat(msg, ss);
    sprintf(ss, "(old HSLB firmware < 0.46 gives incorrect stats)\n");
    strcat(msg, ss);
  }

  sprintf(ss, "%s", prompt);
  strcat(msg, ss);
  sprintf(ss, "stat=%08x (ff=%x rx=%x pr=%x pt=%x tx=%x%s%s%s%s%s%s%s%s)\n",
          h.hslbsta,
          D(h.hslbsta, 11, 8),
          D(h.hslbsta, 15, 12),
          D(h.hslbsta, 19, 16),
          D(h.hslbsta, 23, 20),
          D(h.hslbsta, 28, 24),
          Bs(h.hslbsta, 0, " linkdown"),
          B(h.hslbsta, 31) && !B(h.hslbsta, 1) ? " linklost" : "",
          Bs(h.hslbsta, 5, " ffstate"),
          Bs(h.hslbsta, 29, " pll"),
          Bs(h.hslbsta, 7, " pll2"),
          Bs(h.hslbsta, 1, " masked"),
          Bs(h.hslbsta, 2, " fful"),
          Bs(h.hslbsta, 4, " nwff"));
  strcat(msg, ss);
  sprintf(state, "%s%s%s%s%s%s%s%s",
          Bs(h.hslbsta, 0, " linkdown"),
          B(h.hslbsta, 31) && !B(h.hslbsta, 1) ? " linklost" : "",
          Bs(h.hslbsta, 5, " ffstate"),
          Bs(h.hslbsta, 29, " pll"),
          Bs(h.hslbsta, 7, " pll2"),
          Bs(h.hslbsta, 1, " masked"),
          Bs(h.hslbsta, 2, " fful"),
          Bs(h.hslbsta, 4, " nwff"));

  set(vname + "b2ldown", (int)B(h.hslbsta, 0));
  set(vname + "b2llost", ((int)(B(h.hslbsta, 31) && !B(h.hslbsta, 1))));
  set(vname + "ffstate", (int)B(h.hslbsta, 5));
  set(vname + "pll", (int)B(h.hslbsta, 29));
  set(vname + "pll2", (int)B(h.hslbsta, 7));
  set(vname + "masked", (int)B(h.hslbsta, 1));
  set(vname + "fful", (int)B(h.hslbsta, 2));
  set(vname + "nwff", (int)B(h.hslbsta, 4));

  sprintf(ss, "%s", prompt);
  strcat(msg, ss);
  sprintf(ss, "rxdata=%04x rxlinkdown=%d rxcrcerr=%d feecrcerr=%d\n",
          D(h.rxdata, 15, 0), D(h.rxdata, 31, 24), D(h.rxdata, 23, 16),
          h.feecrce);
  strcat(msg, ss);
  set(vname + "rxdata", (int)D(h.rxdata, 15, 0));
  set(vname + "rxldown", (int)D(h.rxdata, 31, 24));
  set(vname + "rxcrce", (int)D(h.rxdata, 23, 16));
  set(vname + "feecrce", (int)h.feecrce);

  sprintf(ss, "%s", prompt);
  strcat(msg, ss);
  total = (h.nkbyte * 256.0 + (h.nword & 0xff)) * 4.0;
  sprintf(ss, "event=%d total=%1.0fkB", h.nevent, total / 1000);
  strcat(msg, ss);
  set(vname + "nevent", (int)h.nevent);
  set(vname + "kbypes", (int)(total / 1024));
  if (h.nevent) {
    double avg = total / h.nevent;
    sprintf(ss, " (avg=%1.0fB", avg);
    strcat(msg, ss);
    set(vname + "evtsz.avg", (int)avg);
    if (D(h.eventsz, 31, 16) == 0xffff) {
      sprintf(ss, " last=oflow");
      strcat(msg, ss);
      set(vname + "evtsz.last", (int) - 1);
    } else {
      sprintf(ss, " last=%dB", D(h.eventsz, 31, 16) * 4);
      strcat(msg, ss);
      set(vname + "evtsz.last", (int)(D(h.eventsz, 31, 16) * 4));
    }
    if (D(h.eventsz, 15, 0) == 0xffff) {
      sprintf(ss, " max=oflow)");
      strcat(msg, ss);
      set(vname + "evtsz.max", (int) - 1);
    } else {
      sprintf(ss, " max=%dB)", D(h.eventsz, 15, 0) * 4);
      strcat(msg, ss);
      set(vname + "evtsz.max", (int)(D(h.eventsz, 15, 0) * 4));
    }
  }
  sprintf(ss, "\n");
  strcat(msg, ss);

  sprintf(ss, "%s", prompt);
  strcat(msg, ss);
  bool iserr = false;
  if ((h.vetoset & 3) == 0) {
    sprintf(ss, "no b2link error correction");
    strcat(msg, ss);
    set(vname + "b2lerr", 0);
  } else if (h.vetocnt == 0) {
    sprintf(ss, "no b2link error");
    strcat(msg, ss);
    set(vname + "b2lerr", 0);
  } else {
    sprintf(ss, "b2link error %d(%c) %d(%c) %c%02x %c%02x %c%02x %c%02x %c%02x %c%02x",
            D(h.vetocnt, 31, 16),
            B(h.vetoset, 1) ? 'i' : '-',
            D(h.vetocnt, 15, 0),
            B(h.vetoset, 0) ? 'd' : '-',
            B(h.vetobuf[0], 25) ? 'K' : 'D', D(h.vetobuf[0], 15, 8),
            B(h.vetobuf[0], 24) ? 'K' : 'D', D(h.vetobuf[0], 7, 0),
            B(h.vetobuf[0], 21) ? 'K' : 'D', D(h.vetobuf[1], 31, 24),
            B(h.vetobuf[0], 20) ? 'K' : 'D', D(h.vetobuf[1], 23, 16),
            B(h.vetobuf[0], 17) ? 'K' : 'D', D(h.vetobuf[1], 15, 8),
            B(h.vetobuf[0], 16) ? 'K' : 'D', D(h.vetobuf[1], 7, 0));
    strcat(msg, ss);
    set(vname + "b2lerr", 1);
    iserr = true;
  }
  if (h.hslbver >= 55) {
    sprintf(ss, " ff00 %d datapkt %d dataword %d",
            h.cntff00, h.cntdatapkt, h.cntdataword);
    strcat(msg, ss);
  }
  sprintf(ss, "\n");
  strcat(msg, ss);
  set(vname + "staths", msg);
  logging(iserr && (getNode().getState() == RCState::RUNNING_S ||
                    getNode().getState() == RCState::LOADING_TS),
          LogFile::ERROR, "HSLB-%c : %s", 'a' + i, state);
  return state;
}
