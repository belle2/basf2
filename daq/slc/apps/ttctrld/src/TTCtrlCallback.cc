#include "daq/slc/apps/ttctrld/TTCtrlCallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/database/DBHandlerException.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/File.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#define USE_LINUX_VME_UNIVERSE 1

#include <ftprogs2/ftstat.h>
#include <ftprogs2/ft2p_fast.h>
#include <ftprogs2/ft2p_slow.h>
#include <ftprogs2/ft2u067.h>
#include <ftprogs2/ft2p026.h>

extern "C" {
  //typedef ft2p_t slow_t;
  //typedef ft2p_t fast_t;
  typedef struct ft2p_fast fast_t;
  typedef struct ft2p_slow slow_t;
  ft2p_t stat2p026(ftsw_t* ftsw, int ftswid, char* ss, char* fstate);
  ft2u_t stat2u067(ftsw_t* ftsw, int ftswid, char* ss, char* fstate);
  void summary2p026(struct timeval* tvp, fast_t* f, slow_t* s, char* ss);
  void color2p026(fast_t* f, slow_t* s, char* ss, char* state);
}

#ifndef D
#define D(a,b,c) (((a)>>(c))&((1<<((b)+1-(c)))-1))
#define B(a,b)   D(a,b,b)
#define Bs(a,b,s)   (B(a,b)?(s):"")
#define Ds(a,b,c,s)   (D(a,b,c)?(s):"")
#endif

#include <sstream>
#include <unistd.h>
#include <sys/time.h>

#define VERSION 20160330

//typedef struct pocket_ttd_fast fast_t;
//typedef struct pocket_ttd      slow_t;

unsigned int prev_u = 0;
unsigned int prev_c = 0;

ftsw_t* g_ftsw = NULL;

Belle2::Mutex g_mutex;
bool g_flag = false;

namespace Belle2 {

  class NSMVHandlerTrigft : public NSMVHandlerInt {
  public:
    NSMVHandlerTrigft(TTCtrlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, false, true), m_callback(callback) {}
    virtual ~NSMVHandlerTrigft() throw() {}
    bool handleSetInt(int val)
    {
      try {
        if (val > 0)
          m_callback.trigft();
        return true;
      } catch (const RCHandlerException& e) {
        LogFile::error(e.what());
      }
      return false;
    }
  private:
    TTCtrlCallback& m_callback;
  };

  class NSMVHandlerStatFT : public NSMVHandlerText {
  public:
    NSMVHandlerStatFT(TTCtrlCallback& callback, const std::string& name)
      : NSMVHandlerText(name, true, false), m_callback(callback) {}
    virtual ~NSMVHandlerStatFT() throw() {}
    bool handleGetText(std::string& val)
    {
      val = g_ftstat.statft;
      return true;
    }
  private:
    TTCtrlCallback& m_callback;
  };

  class NSMVHandlerResetFT : public NSMVHandlerText {
  public:
    NSMVHandlerResetFT(TTCtrlCallback& callback, const std::string& name)
      : NSMVHandlerText(name, false, true), m_callback(callback) {}
    virtual ~NSMVHandlerResetFT() throw() {}
    bool handleSetText(const std::string& val)
    {
      if (val == "on")
        m_callback.resetft();
      return true;
    }
  private:
    TTCtrlCallback& m_callback;
  };

  class NSMVHandlerStartFT : public NSMVHandlerInt {
  public:
    NSMVHandlerStartFT(TTCtrlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, false, true), m_callback(callback) {}
    virtual ~NSMVHandlerStartFT() throw() {}
    bool handleSetInt(int val)
    {
      g_flag = true;
      int expno = (val >> 24) & 0xff;
      int runno = val & 0xffffff;
      m_callback.resetft();
      usleep(1000);
      m_callback.start(expno, runno);
      return true;
    }
  private:
    TTCtrlCallback& m_callback;
  };

  class NSMVHandlerTrigIO : public NSMVHandlerText {
  public:
    NSMVHandlerTrigIO(TTCtrlCallback& callback, const std::string& name)
      : NSMVHandlerText(name, true, true, std::string("off")), m_callback(callback) {}
    virtual ~NSMVHandlerTrigIO() throw() {}
    bool handleSetText(const std::string& val)
    {
      m_callback.trigio(val);
      return true;
    }
  private:
    TTCtrlCallback& m_callback;
  };

  class NSMVHandlerRegFT : public NSMVHandlerInt {
  public:
    NSMVHandlerRegFT(TTCtrlCallback& callback, const std::string& name, unsigned int addr)
      : NSMVHandlerInt(name, true, true, 0), m_callback(callback), m_addr(addr) {}
    virtual ~NSMVHandlerRegFT() throw() {}
    bool handleSetInt(int val)
    {
      unsigned int offset = m_addr >> 2;
      write_ftsw(g_ftsw, offset, val);
      return true;
    }
    bool handleGetInt(int& val)
    {
      unsigned int offset = m_addr >> 2;
      val = read_ftsw(g_ftsw, offset);
      return true;
    }
  private:
    TTCtrlCallback& m_callback;
    unsigned int m_addr;
  };

  class NSMVHandlerMaxTimeFT : public NSMVHandlerFloat {
  public:
    NSMVHandlerMaxTimeFT(TTCtrlCallback& callback, const std::string& name)
      : NSMVHandlerFloat(name, true, true, 0), m_callback(callback) {}
    virtual ~NSMVHandlerMaxTimeFT() throw() {}
    bool handleSetFloat(float usec)
    {
      unsigned int offset = 0x500 >> 2;
      int v = read_ftsw(g_ftsw, offset);
      unsigned int val = (v & 0xFF000000) | (unsigned int)(usec * 1e+3 / 7.8);
      write_ftsw(g_ftsw, offset, val);
      return true;
    }
    bool handleGetFloat(float& val)
    {
      unsigned int addr = 0x500;
      unsigned int offset = addr >> 2;
      int v = read_ftsw(g_ftsw, offset);
      val = (v & 0xFFFFFF) * 7.8e-3;
      return true;
    }
  private:
    TTCtrlCallback& m_callback;
  };

  class NSMVHandlerMaxTrigFT : public NSMVHandlerInt {
  public:
    NSMVHandlerMaxTrigFT(TTCtrlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, true, 0), m_callback(callback) {}
    virtual ~NSMVHandlerMaxTrigFT() throw() {}
    bool handleSetInt(int maxtrg)
    {
      unsigned int offset = 0x500 >> 2;
      int v = read_ftsw(g_ftsw, offset);
      unsigned int val = (v & 0xFFFFFF) | (maxtrg << 24);
      write_ftsw(g_ftsw, offset, val);
      return true;
    }
    bool handleGetInt(int& val)
    {
      unsigned int addr = 0x500;
      unsigned int offset = addr >> 2;
      int v = read_ftsw(g_ftsw, offset);
      val = v >> 24;
      return true;
    }
  private:
    TTCtrlCallback& m_callback;
  };

  class NSMVHandlerCmdFT : public NSMVHandlerInt {
  public:
    NSMVHandlerCmdFT(TTCtrlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, true, 0), m_callback(callback) {}
    virtual ~NSMVHandlerCmdFT() throw() {}
    bool handleSetInt(int val)
    {
      uint32_t cmdhi = val & 0xFF;
      uint32_t cmdlo = (val >> 16) & 0xFF;
      write_ftsw(g_ftsw, FTSWREG(0x18), cmdhi);
      write_ftsw(g_ftsw, FTSWREG(0x19), cmdlo);
      return true;
    }
  private:
    TTCtrlCallback& m_callback;
  };


  class NSMVHandlerPortFT : public NSMVHandlerInt {
  public:
    NSMVHandlerPortFT(TTCtrlCallback& callback, const std::string& name, int enable)
      : NSMVHandlerInt(name, true, true, enable), m_callback(callback) {}
    virtual ~NSMVHandlerPortFT() throw() {}
    bool handleSetInt(int val)
    {
      NSMVHandlerInt::handleSetInt(val);
      val = 0;
      for (int i = 0; i < 4; i++) {
        int used = 0;
        std::string vname = StringUtil::form("port.cpr[%d].enable", i);
        m_callback.get(vname, used);
        used = !used;
        val += (used << (12 + i));
      }
      for (int i = 0; i < 8; i++) {
        int used = 0;
        std::string vname = StringUtil::form("port.fee[%d].enable", i);
        m_callback.get(vname, used);
        used = !used;
        val += (used << i);
      }
      unsigned int offset = 0x170 >> 2;
      write_ftsw(g_ftsw, offset, val);
      return true;
    }
  private:
    TTCtrlCallback& m_callback;
  };

  class NSMVHandlerJTAGEnable : public NSMVHandlerInt {
  public:
    NSMVHandlerJTAGEnable(TTCtrlCallback& callback, const std::string& name, int enable)
      : NSMVHandlerInt(name, true, true, enable), m_callback(callback) {}
    virtual ~NSMVHandlerJTAGEnable() throw() {}
    bool handleSetInt(int val)
    {
      NSMVHandlerInt::handleSetInt(val);
      val = 0;
      for (int i = 0; i < 8; i++) {
        int used = 0;
        std::string vname = StringUtil::form("jtag.fee[%d].enable", i);
        m_callback.get(vname, used);
        val += (used << i);
      }
      unsigned int offset = 0x1a0 >> 2;
      write_ftsw(g_ftsw, offset, val);
      return true;
    }
  private:
    TTCtrlCallback& m_callback;
  };

  class NSMVHandlerJTAGFT : public NSMVHandlerText {
  public:
    NSMVHandlerJTAGFT(TTCtrlCallback& callback, const std::string& name,
                      int ftswid, const std::string& firmware)
      : NSMVHandlerText(name, true, true, firmware), m_callback(callback), m_ftswid(ftswid) {}
    virtual ~NSMVHandlerJTAGFT() throw() {}
    bool handleSetText(const std::string& firmware)
    {
      if (File::exist(firmware)) {
        NSMVHandlerText::handleSetText(firmware);
        std::stringstream ss;
        for (int i = 0; i < 8; i++) {
          int used = 0;
          std::string vname = StringUtil::form("jtag.fee[%d].enable", i);
          m_callback.get(vname, used);
          if (used) {
            ss << "-p" << i << " ";
          }
        }
        std::string cmd = StringUtil::form("jtagft -%d %s -tfp program ", m_ftswid, ss.str().c_str()) + firmware;
        LogFile::info(cmd);
        m_callback.set("msg", "jtagft to " + ss.str());
        m_callback.log(LogFile::INFO, "jtagft to " + ss.str());
        system(cmd.c_str());
        m_callback.set("msg", "jtagft done");
        m_callback.log(LogFile::INFO, "jtagft done: %s", firmware.c_str());
        return true;
      } else {
        m_callback.log(LogFile::WARNING, "Ignored : No bit file %s", firmware.c_str());
        m_callback.set("msg", "No bit file: " + firmware);
      }
      return false;
    }
  private:
    TTCtrlCallback& m_callback;
    int m_ftswid;
  };

  class TriggerLimit {
  public:
    TriggerLimit(TTCtrlCallback& callback, int ftswid)
      : m_callback(callback), m_ftswid(ftswid) {}
  public:
    void run()
    {
      char ss[1024];
      while (true) {
        g_mutex.lock();
        try {
          if (g_flag == true) {
            stat2u067(g_ftsw, m_ftswid, ss, g_ftstat.state);
            std::string state = g_ftstat.state;
            if (state == "READY"
                //(state == "READY" || state == "PAUSED")
                && g_ftstat.toutcnt >= g_ftstat.tlimit) {
              m_callback.log(LogFile::NOTICE, "Trigger limit (state = %s)", g_ftstat.state);
              m_callback.set("toutcnt", (int)g_ftstat.toutcnt);
              m_callback.set("ftstate", g_ftstat.state);
              m_callback.set("endft", (int)1);
              g_flag = false;
            }
          }
        } catch (const NSMHandlerException& e) {
          LogFile::error(e.what());
        }
        g_mutex.unlock();
        usleep(1000);
      }
    }
  private:
    TTCtrlCallback& m_callback;
    int m_ftswid;
  };

  std::string popen(const std::string& cmd)
  {
    char buf[1000];
    FILE* fp;
    if ((fp = ::popen(cmd.c_str(), "r")) == NULL) {
      LogFile::warning("can not exec commad");
      return "";
    }
    std::stringstream ss;
    while (!feof(fp)) {
      memset(buf, 0, 1000);
      fgets(buf, sizeof(buf), fp);
      ss << buf;
    }
    pclose(fp);
    return ss.str();
  }
}

using namespace Belle2;

TTCtrlCallback::TTCtrlCallback(int ftswid, const std::string& ttdname)
  : RCCallback(4), m_ftswid(ftswid), m_ttdnode(ttdname)
{
  m_trgcommands.insert(std::map<std::string, int>::value_type("none", 0));
  m_trgcommands.insert(std::map<std::string, int>::value_type("aux", 1));
  m_trgcommands.insert(std::map<std::string, int>::value_type("i", 2));
  m_trgcommands.insert(std::map<std::string, int>::value_type("tlu", 3));
  m_trgcommands.insert(std::map<std::string, int>::value_type("pulse", 4));
  m_trgcommands.insert(std::map<std::string, int>::value_type("revo", 5));
  m_trgcommands.insert(std::map<std::string, int>::value_type("random", 6));
  m_trgcommands.insert(std::map<std::string, int>::value_type("poisson", 7));
  m_trgcommands.insert(std::map<std::string, int>::value_type("once", 8));
  m_trgcommands.insert(std::map<std::string, int>::value_type("stop", 9));
  memset(&g_ftstat, 0, sizeof(ftstat_t));
}

void TTCtrlCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  std::string ttdname = m_ttdnode.getName();
  if (ttdname.size() > 0) {
    openData(ttdname + "FAST", "ft2p_fast", ft2p_fast_revision);
    openData(ttdname + "SLOW", "ft2p_slow", ft2p_slow_revision);
  }
  setAutoReply(true);
  if (g_ftsw == NULL) {
    LogFile::debug("ftsw = %d", m_ftswid);
    g_ftsw = open_ftsw(m_ftswid, FTSW_RDWR);
    if (g_ftsw == NULL) {
      LogFile::error("failed to open ftsw");
    }
  }
  configure(obj);
  g_flag = false;
  PThread(new TriggerLimit(*this, m_ftswid));
}

void TTCtrlCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  add(new NSMVHandlerText("msg", true, false, ""));
  add(new NSMVHandlerInt("expno", true, false, 0));
  add(new NSMVHandlerInt("runno", true, false, 0));
  add(new NSMVHandlerInt("subno", true, false, 0));
  add(new NSMVHandlerText("busy", true, false));
  add(new NSMVHandlerText("reset", true, false));
  add(new NSMVHandlerText("stafifo", true, false));
  add(new NSMVHandlerInt("tincnt", true, false, 0));
  add(new NSMVHandlerInt("toutcnt", true, false, 0));
  add(new NSMVHandlerInt("atrigc", true, false, 0));
  add(new NSMVHandlerFloat("rateall", true, false, 0));
  add(new NSMVHandlerFloat("raterun", true, false, 0));
  add(new NSMVHandlerFloat("rateout", true, false, 0));
  add(new NSMVHandlerFloat("ratein", true, false, 0));
  add(new NSMVHandlerInt("tlimit", true, false, 0));
  add(new NSMVHandlerInt("tlast", true, false, 0));
  add(new NSMVHandlerText("err", true, false));
  add(new NSMVHandlerText("errport", true, false));
  add(new NSMVHandlerInt("tstart", true, false, 0));
  add(new NSMVHandlerText("tstart_s", true, false, "----"));
  add(new NSMVHandlerInt("trun", true, false, 0));
  add(new NSMVHandlerText("ftstate", true, false, "UNKNOWN"));
  add(new NSMVHandlerText("statft", true, false, ""));
  //add(new NSMVHandlerStatFT(*this, "statft"));
  add(new NSMVHandlerResetFT(*this, "resetft"));
  add(new NSMVHandlerStartFT(*this, "startft"));
  add(new NSMVHandlerTrigIO(*this, "trigio"));
  add(new NSMVHandlerInt("endft", true, false, 0));
  add(new NSMVHandlerRegFT(*this, "portmask", 0x170));
  add(new NSMVHandlerRegFT(*this, "jtagmask", 0x1a0));
  add(new NSMVHandlerMaxTimeFT(*this, "maxtime"));
  add(new NSMVHandlerMaxTrigFT(*this, "maxtrig"));
  add(new NSMVHandlerCmdFT(*this, "cmdft"));
  add(new NSMVHandlerJTAGFT(*this, "jtagft", m_ftswid, ""));

  const DBObject& o_port(obj("port"));
  if (o_port.hasObject("fee")) {
    const DBObjectList& o_fees(o_port.getObjects("fee"));
    add(new NSMVHandlerInt("nfees", true, false, (int)o_fees.size()));
    for (unsigned int i = 0; i < o_fees.size(); i++) {
      int enable = obj("port")("fee", i).getBool("enable");
      add(new NSMVHandlerPortFT(*this, StringUtil::form("port.fee[%d].enable", i), enable));
      enable = obj("jtag")("fee", i).getBool("enable");
      add(new NSMVHandlerJTAGEnable(*this, StringUtil::form("jtag.fee[%d].enable", i), enable));
      std::string vname = StringUtil::form("link.o[%d].", i);
      add(new NSMVHandlerInt(vname + "linkup", true, false, 0));
      add(new NSMVHandlerInt(vname + "mask", true, false, 0));
      add(new NSMVHandlerInt(vname + "ttlost", true, false, 0));
      add(new NSMVHandlerInt(vname + "ttldn", true, false, 0));
      add(new NSMVHandlerInt(vname + "b2lost", true, false, 0));
      add(new NSMVHandlerInt(vname + "b2ldn", true, false, 0));
      add(new NSMVHandlerInt(vname + "busy", true, false, 0));
      add(new NSMVHandlerInt(vname + "feeerr", true, false, 0));
      add(new NSMVHandlerInt(vname + "tagerr", true, false, 0));
      add(new NSMVHandlerInt(vname + "fifoerr", true, false, 0));
      add(new NSMVHandlerInt(vname + "seu", true, false, 0));
    }
  } else {
    add(new NSMVHandlerInt("nfees", true, false, 0));
  }
  if (o_port.hasObject("cpr")) {
    const DBObjectList& o_cprs(o_port.getObjects("cpr"));
    add(new NSMVHandlerInt("ncprs", true, false, (int)o_cprs.size()));
    for (unsigned int i = 0; i < o_cprs.size(); i++) {
      int enable = o_cprs[i].getBool("enable");
      add(new NSMVHandlerPortFT(*this, StringUtil::form("port.cpr[%d].enable", i), enable));
      std::string vname = StringUtil::form("link.x[%d].", i);
      add(new NSMVHandlerInt(vname + "linkup", true, false, 0));
      add(new NSMVHandlerInt(vname + "mask", true, false, 0));
      add(new NSMVHandlerInt(vname + "err", true, false, 0));
      for (int j = 0; j < 4; j++) {
        std::string vname = StringUtil::form("link.x[%d].%c.", i, j + 'a');
        add(new NSMVHandlerInt(vname + "enable", true, false, 0));
        add(new NSMVHandlerInt(vname + "empty", true, false, 0));
        add(new NSMVHandlerInt(vname + "full", true, false, 0));
        add(new NSMVHandlerInt(vname + "err", true, false, 0));
      }
    }
  } else {
    add(new NSMVHandlerInt("ncprs", true, false, 0));
  }
  set("ftsw", obj.getInt("ftsw"));
  set("dummy_rate", obj.getInt("dummy_rate"));
  set("trigger_limit", obj.getInt("trigger_limit"));
  set("trigger_type", obj.getText("trigger_type"));
  monitor();
}

void TTCtrlCallback::monitor() throw(RCHandlerException)
{
  g_mutex.lock();
  if (m_ttdnode.getName().size() > 0) {
    send(m_ttdnode, NSMMessage(m_ttdnode, NSMCommand(17, "STATECHECK")));
  }
  statftx(g_ftsw, m_ftswid);
  g_mutex.unlock();
}

void TTCtrlCallback::boot(const DBObject& obj) throw(RCHandlerException)
{
  resetft();
  abort();
  setState(RCState::NOTREADY_S);
}

void TTCtrlCallback::load(const DBObject&) throw(RCHandlerException)
{
  resetft();
  trigft();
  if (!g_flag && m_ttdnode.getName().size() > 0) {
    send(m_ttdnode, NSMMessage(m_ttdnode, NSMCommand(13, "LOAD")));
  }
  setState(RCState::READY_S);
}

void TTCtrlCallback::start(int expno, int runno) throw(RCHandlerException)
{
  DBObject& obj(getDBObject());
  get(obj);
  int dummy_rate = obj.getInt("dummy_rate");
  int trigger_limit = obj.getInt("trigger_limit");
  std::string trigger_type = obj.getText("trigger_type");
  g_mutex.lock();
  if (!g_flag && m_ttdnode.getName().size() > 0) {
    int pars[3];
    pars[0] = m_trgcommands[trigger_type];
    pars[1] = dummy_rate * 1000;
    pars[2] = trigger_limit;
    send(m_ttdnode, NSMMessage(m_ttdnode, NSMCommand(11, "TRIGFT"), 3, pars));
    pars[0] = expno;
    pars[1] = runno;
    send(m_ttdnode, NSMMessage(m_ttdnode, NSMCommand(12, "START"), 2, pars));
  } else {
    DBObject& obj(getDBObject());
    get(obj);
    int dummy_rate = obj.getInt("dummy_rate");
    int trigger_limit = obj.getInt("trigger_limit");
    std::string trigger_type = obj.getText("trigger_type");
    std::string cmd = StringUtil::form("regft -%d 160 0x%x", m_ftswid, (expno << 22) + ((runno - 1) << 8));
    LogFile::debug(cmd);
    system(cmd.c_str());
    usleep(1000);
    if (trigger_type == "aux") {
      cmd = StringUtil::form("trigft -%d %s %d -1", m_ftswid,
                             trigger_type.c_str(), trigger_limit);
    } else {
      cmd = StringUtil::form("trigft -%d %s %d %d", m_ftswid,
                             trigger_type.c_str(), dummy_rate, trigger_limit);
    }
    LogFile::debug(cmd);
    system(cmd.c_str());
  }
  g_mutex.unlock();
  monitor();
}

void TTCtrlCallback::stop() throw(RCHandlerException)
{
  if (m_ttdnode.getName().size() > 0 && m_ttdnode.getState() == RCState::RUNNING_S) {
    send(m_ttdnode, NSMMessage(m_ttdnode, NSMCommand(13, "STOP")));
  } else {
    std::string cmd = StringUtil::form("trigft -%d reset", m_ftswid);
    LogFile::debug(cmd);
    system(cmd.c_str());
  }
}

bool TTCtrlCallback::pause() throw(RCHandlerException)
{
  if (m_ttdnode.getName().size() > 0) {
    send(m_ttdnode, NSMMessage(m_ttdnode, NSMCommand(14, "PAUSE")));
  }
  return true;
}

bool TTCtrlCallback::resume(int /*subno*/) throw(RCHandlerException)
{
  if (m_ttdnode.getName().size() > 0) {
    send(m_ttdnode, NSMMessage(m_ttdnode, NSMCommand(15, "RESUME")));
  }
  return true;
}

void TTCtrlCallback::recover(const DBObject&) throw(RCHandlerException)
{
  if (m_ttdnode.getName().size() > 0) {
    send(m_ttdnode, NSMMessage(m_ttdnode, NSMCommand(16, "RECOVER")));
  }
}

void TTCtrlCallback::abort() throw(RCHandlerException)
{
  stop();
  setState(RCState::NOTREADY_S);
}

void TTCtrlCallback::trigft() throw(RCHandlerException)
{
  g_mutex.lock();
  try {
    DBObject& obj(getDBObject());
    get(obj);
    int ftswid = obj.getInt("ftsw");
    int dummy_rate = obj.getInt("dummy_rate");
    int trigger_limit = obj.getInt("trigger_limit");
    std::string trigger_type = obj.getText("trigger_type");
    int pars[4];
    pars[0] = m_trgcommands[trigger_type];
    pars[1] *= 1000;
    pars[2] = trigger_limit;
    pars[3] = dummy_rate;
    if (!g_flag && m_ttdnode.getName().size() > 0) {
      send(m_ttdnode, NSMMessage(m_ttdnode, NSMCommand(11, "TRIGFT"), 4, pars));
    } else {
      std::string cmd = StringUtil::form("trigft -%d reset", ftswid);
      LogFile::debug(cmd);
      system(cmd.c_str());
      sleep(1);
    }
  } catch (const std::out_of_range& e) {
    LogFile::error(e.what());
  }
  g_mutex.unlock();
}

void TTCtrlCallback::trigio(const std::string& type) throw(RCHandlerException)
{
  try {
    DBObject& obj(getDBObject());
    get(obj);
    int dummy_rate = obj.getInt("dummy_rate");
    int trigger_limit = obj.getInt("trigger_limit");
    std::string cmd;
    if (type == "on") {
      cmd = StringUtil::form("trigio -localbusy -fifo=no pulse %d %d",
                             dummy_rate, trigger_limit);
    } else {
      cmd = "trigio -localbusy -fifo=no clear";
    }
    LogFile::debug(cmd);
    system(cmd.c_str());
    sleep(1);
  } catch (const std::out_of_range& e) {
    LogFile::error(e.what());
  }
}

void TTCtrlCallback::send(RCNode& node, const NSMMessage& msg)
throw(RCHandlerException)
{
  RCState cstate(node.getState());
  try {
    NSMCommunicator::connected(node.getName());
    NSMCommunicator::send(msg);
  } catch (const NSMNotConnectedException&) {
    if (cstate != Enum::UNKNOWN) {
      node.setState(Enum::UNKNOWN);
      setState(RCState::ERROR_ES);
      throw (RCHandlerException(node.getName() + " is down"));
    }
  } catch (const NSMHandlerException& e) {
    setState(RCState::ERROR_ES);
    throw (RCHandlerException(e.what()));
  }
}

void TTCtrlCallback::ok(const char* nodename, const char* data) throw()
{
  if (m_ttdnode.getName().size() > 0 &&
      m_ttdnode.getName() == nodename) {
    LogFile::debug("OK from %s (state = %s)", nodename, data);
    try {
      RCState state(data);
      m_ttdnode.setState(state);
      if (state == RCState::RUNNING_S) {
        setState(state);
      } else {
        state = RCState(getNode().getState());
        state = state.next();
        if (state != RCState::UNKNOWN) {
          setState(state);
        }
      }
    } catch (const std::out_of_range& e) {}
  }
}

void TTCtrlCallback::error(const char* nodename, const char* data) throw()
{
  if (m_ttdnode.getName().size() > 0 &&
      m_ttdnode.getName() == nodename) {
    log(LogFile::WARNING, "ERROR from %s : %s", nodename, data);
    try {
      //setState(RCState::NOTREADY_S);
    } catch (const std::out_of_range& e) {}
  }
}

void TTCtrlCallback::resetft() throw()
{
  g_mutex.lock();
  if (!g_flag && m_ttdnode.getName().size() > 0) {
    send(m_ttdnode, NSMMessage(m_ttdnode, NSMCommand(17, "RESETFT")));
  } else {
    DBObject& obj(getDBObject());
    get(obj);
    int ftswid = obj.getInt("ftsw");
    std::string cmd = StringUtil::form("resetft -%d", ftswid);
    LogFile::info(cmd);
    system(cmd.c_str());
  }
  g_mutex.unlock();
}

void TTCtrlCallback::statftx(ftsw_t* ftsw, int ftswid)
{
  char ss [2000];
  ss[0] = 0;
  // basic FTSW check
  int cpldid  = read_ftsw(ftsw, FTSWREG_FTSWID);
  int cpldver = read_ftsw(ftsw, FTSWREG_CPLDVER) & 0xffff;
  int fpgaid  = read_ftsw(ftsw, FTSWREG_FPGAID);
  int ftidhi  = fpgaid & 0xfffffe00;
  int ftidlo  = fpgaid & 0xff;
  int fpgaver = read_ftsw(ftsw, FTSWREG_FPGAVER) & 0x3ff;
  int conf    = read_ftsw(ftsw, FTSWREG_CONF);

  if (cpldid != 0x46545357) {
    LogFile::fatal("FTSW#%03d not found: id=%08x\n", ftswid, cpldid);
    return;
  }
  if (ftswid >= 8 && ftswid < 100 && cpldver < 46) {
    LogFile::fatal("old Spartan-3AN firmware version (%d.%d) found for FTSW2",
                   cpldver / 100, cpldver % 100);
    return;
  }

  if (!(conf & 0x80)) {
    LogFile::fatal("FPGA is not programmed (DONE is not high).");
    return;
  }

  static double toutcnt_old = 0;
  static double tincnt_old = 0;
  static double utime_old = 0;
  char fstate[100];
  if (ftidhi != 0x46543200) {
    ftidhi = 0;
    /*
    } else if (ftidlo == 'U' && fpgaver >= 67) {
    ft2u_t u = stat2u067(ftsw, ftswid, ss, fstate);
    int exp = D(u.exprun, 31, 22);
    int run = D(u.exprun, 21, 8);
    int sub = D(u.exprun, 7, 0);
    double dt = u.utime;
    dt -=  utime_old;
    if (dt > 0) {
      float rateout = (double)(u.toutcnt - toutcnt_old) / dt;
      float ratein = (double)(u.tincnt - tincnt_old) / dt;
      set("rateout", rateout);
      set("ratein", ratein);
      toutcnt_old = u.toutcnt;
      tincnt_old = u.tincnt;
      utime_old = u.utime;
    }
    set("expno", exp);
    set("runno", run);
    set("subno", sub);
    set("tincnt", (int)u.tincnt);
    set("toutcnt", (int)u.toutcnt);
    set("tlimit", (int)u.tlimit);
    set("tlast", (int)u.tlast);
    set("tstart_s", Date(u.rstutim).toString());
    set("tstart", (int)u.rstutim);
    set("trun", (int)(u.utime - u.rstutim));
    set("ftstate", fstate);
    //set("statft", ss);
    std::string s_statft = popen(StringUtil::form("statft -%d", ftswid));
    if (s_statft.size() > 0) {
      set("statft", s_statft);
    }
    const DBObject& o_port(getDBObject()("port"));
    if (o_port.hasObject("fee")) {
      const DBObjectList& o_fees(o_port.getObjects("fee"));
      for (unsigned int i = 0; i < o_fees.size(); i++) {
        std::string vname = StringUtil::form("link.o[%d].", i);
        int up = B(u.linkup, i);
        set(vname + "linkup", up);
    int mask = (int)B(u.omask, i);
    set(vname + "mask", mask);
    if (up && (!mask)) {
          if (B(u.odata[i], 1)) {
            set(vname + "ttlost", (int)B(u.odata[i], 10));
            set(vname + "ttldn", 0);
          } else {
            set(vname + "ttlost", 0);
            set(vname + "ttldn", 1);
          }
          if (B(u.odata[i], 3)) {
            set(vname + "b2lost", (int)B(u.odata[i], 9));
            set(vname + "b2ldn", 0);
          } else {
            set(vname + "b2lost", 0);
            set(vname + "b2ldn", 1);
          }
          set(vname + "busy", (int)(D(u.odata[i], 11, 4) ? 0 : B(u.odatb[i], 31)));
          set(vname + "feeerr", (int)B(u.odata[i], 11));
          set(vname + "tagerr", (int)B(u.odata[i], 8));
          set(vname + "fifoerr", (int)D(u.odata[i], 7, 6));
          set(vname + "seu", (int)D(u.odata[i], 5, 4));
        } else {
          set(vname + "mask", 1);
          set(vname + "ttldn", 0);
          set(vname + "ttlost", 0);
          set(vname + "b2lost", 0);
          set(vname + "b2ldn", 0);
          set(vname + "tagerr", 0);
          set(vname + "fifoerr", 0);
          set(vname + "seu", 0);
        }
      }
      if (o_port.hasObject("cpr")) {
        const DBObjectList& o_cprs(o_port.getObjects("cpr"));
        for (unsigned int i = 0; i < o_cprs.size(); i++) {
          std::string vname = StringUtil::form("link.x[%d].", i);
          int up = B(u.linkup, i + 8);
          set(vname + "linkup", up);
          if (up) {
            set(vname + "mask", (int)(!D(u.xdata[i], 31, 28)));
            set(vname + "err", (int)D(u.xdata[i], 27, 24));
            set(vname + "a.enable", (int)B(u.xdata[i], 28));
            set(vname + "b.enable", (int)B(u.xdata[i], 29));
            set(vname + "c.enable", (int)B(u.xdata[i], 30));
            set(vname + "d.enable", (int)B(u.xdata[i], 31));
            set(vname + "a.empty", (int)B(u.xdata[i], 16));
            set(vname + "b.empty", (int)B(u.xdata[i], 17));
            set(vname + "c.empty", (int)B(u.xdata[i], 18));
            set(vname + "d.empty", (int)B(u.xdata[i], 19));
            set(vname + "a.full", (int)B(u.xdata[i], 20));
            set(vname + "b.full", (int)B(u.xdata[i], 21));
            set(vname + "c.full", (int)B(u.xdata[i], 22));
            set(vname + "d.full", (int)B(u.xdata[i], 23));
            set(vname + "a.err", (int)B(u.xdata[i], 24));
            set(vname + "b.err", (int)B(u.xdata[i], 25));
            set(vname + "c.err", (int)B(u.xdata[i], 26));
            set(vname + "d.err", (int)B(u.xdata[i], 27));
          } else {
      set(vname + "mask", 1);
      set(vname + "err", 0);
      for (int j = 0; j < 4; j++) {
        std::string vname = StringUtil::form("link.x[%d].%c.", i, j + 'a');
        set(vname + "enable", 0);
        set(vname + "empty", 0);
        set(vname + "full", 0);
        set(vname + "err", 0);
            }
          }
        }
      }
    }
    prev_u = u.utime;
    prev_c = u.ctime;
    } else if ((ftidlo == 'O' && fpgaver >= 26) ||
             (ftidlo == 'P' && fpgaver >= 26)) {
    */
  } else {
    std::string ttdname = m_ttdnode.getName();
    NSMData& dfast(getData(ttdname + "FAST"));
    NSMData& dslow(getData(ttdname + "SLOW"));
    if (!dfast.isAvailable()) {
      LogFile::debug(ttdname + "FAST is not available");
      return;
    }
    if (!dslow.isAvailable()) {
      LogFile::debug(ttdname + "SLOW is not available");
      return;
    }
    ft2p_fast* f = (ft2p_fast*)dfast.get();
    ft2p_slow* s = (ft2p_slow*)dslow.get();
    struct timeval tv;
    color2p026(f, s, ss, fstate);
    summary2p026(&tv, f, s, ss);
    //ft2p_t p = stat2p026(ftsw, ftswid, ss, fstate);
    int exp = D(s->exprun, 31, 22);
    int run = D(s->exprun, 21, 8);
    int sub = D(s->exprun, 7, 0);
    double dt = f->utime;
    dt -=  utime_old;
    if (dt > 0) {
      float rateout = (double)(f->toutcnt - toutcnt_old) / dt;
      float ratein = (double)(f->tincnt - tincnt_old) / dt;
      set("rateout", rateout);
      set("ratein", ratein);
      toutcnt_old = f->toutcnt;
      tincnt_old = f->tincnt;
      utime_old = f->utime;
    }
    set("expno", exp);
    set("runno", run);
    set("subno", sub);
    set("tincnt", (int)f->tincnt);
    set("toutcnt", (int)f->toutcnt);
    set("tlimit", (int)s->tlimit);
    set("tlast", (int)f->tlast);
    set("tstart_s", Date(s->rstutim).toString());
    set("tstart", (int)s->rstutim);
    set("trun", (int)(f->utime - s->rstutim));
    set("ftstate", fstate);
    std::string s_statft = popen(StringUtil::form("statft -%d", ftswid));
    if (s_statft.size() > 0) {
      set("statft", s_statft);
    }
    const DBObject& o_port(getDBObject()("port"));
    if (o_port.hasObject("fee")) {
      const DBObjectList& o_fees(o_port.getObjects("fee"));
      for (unsigned int i = 0; i < o_fees.size(); i++) {
        std::string vname = StringUtil::form("link.o[%d].", i);
        int up = B(f->oalive, i);
        set(vname + "linkup", up);
        int mask = (int)B(s->omask, i);
        set(vname + "mask", mask);
        if (up && !mask) {
          int fa = f->odata[i];
          int fb = f->odatb[i];
          //int fc = f->odatc[i];
          if (B(fa, 1)) {
            set(vname + "ttlost", (int)B(fa, 10));
            set(vname + "ttldn", 0);
          } else {
            set(vname + "ttlost", 0);
            set(vname + "ttldn", 1);
          }
          if (B(fa, 3)) {
            set(vname + "b2lost", (int)B(fa, 9));
            set(vname + "b2ldn", 0);
          } else {
            set(vname + "b2lost", 0);
            set(vname + "b2ldn", 1);
          }
          set(vname + "busy", (int)(D(fa, 11, 4) ? 0 : B(fb, 31)));
          set(vname + "feeerr", (int)B(fa, 11));
          set(vname + "tagerr", (int)B(fa, 8));
          set(vname + "fifoerr", (int)D(fa, 7, 6));
          set(vname + "seu", (int)D(fa, 5, 4));
        } else {
          set(vname + "mask", 1);
          set(vname + "ttldn", 0);
          set(vname + "ttlost", 0);
          set(vname + "b2lost", 0);
          set(vname + "b2ldn", 0);
          set(vname + "busy", 0);
          set(vname + "feeerr", 0);
          set(vname + "tagerr", 0);
          set(vname + "fifoerr", 0);
          set(vname + "seu", 0);
        }
      }
    }
    prev_u = f->utime;
    prev_c = f->ctime;
  }
  //printf(ss);

  if (ftidhi == 0) {
    char fpgastr[5];
    int isfpgastr = 1;
    int i;
    for (i = 0; i < 4; i++) {
      fpgastr[i] = D(fpgaid, (3 - i) * 8 + 7, (3 - i) * 8);
      if (! isprint(fpgastr[i])) isfpgastr = 0;
    }
    fpgastr[4] = 0;
    if (isfpgastr) {
      LogFile::fatal("unsupported FPGA firmware %08x (%s%03d) is programmed.",
                     fpgaid, fpgastr, fpgaver);
    } else {
      LogFile::fatal("unknown FPGA firmware %08x is programmed.", fpgaid);
    }
  }
}
