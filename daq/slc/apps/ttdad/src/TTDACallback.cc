#include "daq/slc/apps/ttdad/TTDACallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#define STATFT_NSM

#include <ftprogs/ftsw.h>
#include <ftprogs/ftstat.h>
#include <ftprogs/pocket_ttd.h>
#include <ftprogs/pocket_ttd_fast.h>
#include <ftprogs/ft2u067.h>

#include <unistd.h>
#include <sys/time.h>

#define VERSION 20160330

typedef struct pocket_ttd_fast fast_t;
typedef struct pocket_ttd      slow_t;

extern "C" {
  void summary2u067(struct timeval* tvp, fast_t* f, slow_t* s);
  void color2u067(fast_t* f, slow_t* s);
  void statft(ftsw_t* ftsw, int ftswid);
  void stat2u067(ftsw_t* ftsw, int ftswid);
}

unsigned int prev_u = 0;
unsigned int prev_c = 0;

ftsw_t* g_ftsw = NULL;

Belle2::Mutex g_mutex;
bool g_flag = false;

namespace Belle2 {

  class NSMVHandlerTrigft : public NSMVHandlerInt {
  public:
    NSMVHandlerTrigft(TTDACallback& callback, const std::string& name)
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
    TTDACallback& m_callback;
  };

  class NSMVHandlerStatFT : public NSMVHandlerText {
  public:
    NSMVHandlerStatFT(TTDACallback& callback, const std::string& name)
      : NSMVHandlerText(name, true, false), m_callback(callback) {}
    virtual ~NSMVHandlerStatFT() throw() {}
    bool handleGetText(std::string& val)
    {
      val = g_ftstat.statft;
      return true;
    }
  private:
    TTDACallback& m_callback;
  };

  class NSMVHandlerResetFT : public NSMVHandlerText {
  public:
    NSMVHandlerResetFT(TTDACallback& callback, const std::string& name)
      : NSMVHandlerText(name, false, true), m_callback(callback) {}
    virtual ~NSMVHandlerResetFT() throw() {}
    bool handleSetText(const std::string& val)
    {
      if (val == "on")
        m_callback.resetft();
      return true;
    }
  private:
    TTDACallback& m_callback;
  };

  class NSMVHandlerStartFT : public NSMVHandlerInt {
  public:
    NSMVHandlerStartFT(TTDACallback& callback, const std::string& name)
      : NSMVHandlerInt(name, false, true), m_callback(callback) {}
    virtual ~NSMVHandlerStartFT() throw() {}
    bool handleSetInt(int val)
    {
      int expno = (val >> 24) & 0xff;
      int runno = val & 0xffffff;
      m_callback.resetft();
      usleep(1000);
      m_callback.start(expno, runno);
      g_flag = true;
      return true;
    }
  private:
    TTDACallback& m_callback;
  };

  class NSMVHandlerTrigIO : public NSMVHandlerText {
  public:
    NSMVHandlerTrigIO(TTDACallback& callback, const std::string& name)
      : NSMVHandlerText(name, false, true, std::string("off")), m_callback(callback) {}
    virtual ~NSMVHandlerTrigIO() throw() {}
    bool handleSetText(const std::string& val)
    {
      m_callback.trigio(val);
      return true;
    }
  private:
    TTDACallback& m_callback;
  };

  class NSMVHandlerRegFT : public NSMVHandlerInt {
  public:
    NSMVHandlerRegFT(TTDACallback& callback, const std::string& name, unsigned int addr)
      : NSMVHandlerInt(name, true, true, 0), m_callback(callback), m_addr(addr) {}
    virtual ~NSMVHandlerRegFT() throw() {}
    bool handleSetInt(int val)
    {
      unsigned int offset = m_addr >> 2;
      write_ftsw(g_ftsw, offset, val);
      LogFile::info("0x%x<<%x", m_addr, val);
      return true;
    }
    bool handleGetInt(int& val)
    {
      unsigned int offset = m_addr >> 2;
      val = read_ftsw(g_ftsw, offset);
      LogFile::info("0x%x>>%x", m_addr, val);
      return true;
    }
  private:
    TTDACallback& m_callback;
    unsigned int m_addr;
  };

  class NSMVHandlerPortFT : public NSMVHandlerInt {
  public:
    NSMVHandlerPortFT(TTDACallback& callback, const std::string& name, int enable)
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
      LogFile::info("0x%x<<%x", 0x170, val);
      return true;
    }
  private:
    TTDACallback& m_callback;
  };

  class NSMVHandlerJTAGFT : public NSMVHandlerInt {
  public:
    NSMVHandlerJTAGFT(TTDACallback& callback, const std::string& name, int enable)
      : NSMVHandlerInt(name, true, true, enable), m_callback(callback) {}
    virtual ~NSMVHandlerJTAGFT() throw() {}
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
      LogFile::info("0x%x<<%x", 0x1a0, val);
      return true;
    }
  private:
    TTDACallback& m_callback;
  };

  class TriggerLimit {
  public:
    TriggerLimit(TTDACallback& callback, int ftswid)
      : m_callback(callback), m_ftswid(ftswid) {}
  public:
    void run()
    {
      while (true) {
        g_mutex.lock();
        try {
          if (g_flag == true) {
            stat2u067(g_ftsw, m_ftswid);
            std::string state = g_ftstat.state;
            if (state == "READY" && g_ftstat.toutcnt >= g_ftstat.tlimit) {
              //m_callback.set("tincnt", (int)g_ftstat.tincnt);
              LogFile::info("%d, state = %s", __LINE__, g_ftstat.state);
              m_callback.set("toutcnt", (int)g_ftstat.toutcnt);
              m_callback.set("ftstate", g_ftstat.state);
              LogFile::info("endft = 1");
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
    TTDACallback& m_callback;
    int m_ftswid;
  };

}

using namespace Belle2;

TTDACallback::TTDACallback() : RCCallback(1)
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

void TTDACallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  int ftswid = obj.getInt("ftsw");
  m_ftswid = ftswid;
  setAutoReply(true);
  if (g_ftsw == NULL) {
    g_ftsw = open_ftsw(ftswid, 0x01);
  }
  configure(obj);
  g_flag = false;
  PThread(new TriggerLimit(*this, m_ftswid));
}

void TTDACallback::configure(const DBObject& obj) throw(RCHandlerException)
{
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
  add(new NSMVHandlerStatFT(*this, "statft"));
  add(new NSMVHandlerResetFT(*this, "resetft"));
  add(new NSMVHandlerStartFT(*this, "startft"));
  add(new NSMVHandlerTrigIO(*this, "trigio"));
  add(new NSMVHandlerInt("endft", true, false, 0));
  add(new NSMVHandlerRegFT(*this, "portmask", 0x170));
  add(new NSMVHandlerRegFT(*this, "jtagmask", 0x1a0));
  for (int i = 0; i < 4; i++) {
    int enable = obj("port")("cpr", i).getBool("enable");
    add(new NSMVHandlerPortFT(*this, StringUtil::form("port.cpr[%d].enable", i), enable));
  }
  for (int i = 0; i < 8; i++) {
    int enable = obj("port")("fee", i).getBool("enable");
    add(new NSMVHandlerPortFT(*this, StringUtil::form("port.fee[%d].enable", i), enable));
  }
  for (int i = 0; i < 8; i++) {
    int enable = obj("jtag")("fee", i).getBool("enable");
    add(new NSMVHandlerJTAGFT(*this, StringUtil::form("jtag.fee[%d].enable", i), enable));
  }
  set("ftsw", obj.getInt("ftsw"));
  set("dummy_rate", obj.getInt("dummy_rate"));
  set("trigger_limit", obj.getInt("trigger_limit"));
  set("trigger_type", obj.getText("trigger_type"));
  monitor();
}

void TTDACallback::monitor() throw(RCHandlerException)
{
  g_mutex.lock();
  try {
    stat2u067(g_ftsw, m_ftswid);
    set("expno", (int)g_ftstat.exp);
    set("runno", (int)g_ftstat.run);
    set("subno", (int)g_ftstat.sub);
    set("busy", g_ftstat.busy);
    set("reset", g_ftstat.reset);
    set("stafifo", g_ftstat.stafifo);
    set("tincnt", (int)g_ftstat.tincnt);
    set("toutcnt", (int)g_ftstat.toutcnt);
    set("atrigc", (float)g_ftstat.atrigc);
    set("rateall", g_ftstat.rateall);
    set("raterun", g_ftstat.raterun);
    set("rateout", g_ftstat.rateout);
    set("ratein", g_ftstat.ratein);
    set("tlimit", (int)g_ftstat.tlimit);
    set("tlast", (int)g_ftstat.tlast);
    set("err", g_ftstat.err);
    set("errport", g_ftstat.errport);
    set("tstart_s", Date(g_ftstat.tstart).toString());
    set("tstart", (int)g_ftstat.tstart);
    set("trun", (int)g_ftstat.trun);
    set("ftstate", g_ftstat.state);
    std::string state = g_ftstat.state;
    if (g_flag) {
      if (state == "READY" && g_ftstat.toutcnt >= g_ftstat.tlimit) {
        //m_callback.set("tincnt", (int)g_ftstat.tincnt);
        LogFile::info("%d, state = %s", __LINE__, g_ftstat.state);
        LogFile::info("endft = 1");
        set("endft", (int)1);
        g_flag = false;
      }
    }
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
  g_mutex.unlock();
}

void TTDACallback::boot(const std::string& opt, const DBObject& obj) throw(RCHandlerException)
{
  resetft();
  abort();
  setState(RCState::NOTREADY_S);
}

void TTDACallback::load(const DBObject&) throw(RCHandlerException)
{
  resetft();
  trigft();
}

void TTDACallback::start(int expno, int runno) throw(RCHandlerException)
{
  DBObject& obj(getDBObject());
  get(obj);
  int ftswid = obj.getInt("ftsw");
  int dummy_rate = obj.getInt("dummy_rate");
  int trigger_limit = obj.getInt("trigger_limit");
  std::string trigger_type = obj.getText("trigger_type");
  std::string cmd = StringUtil::form("regft -%d 160 0x%x", ftswid, (expno << 22) + ((runno - 1) << 8));
  LogFile::debug(cmd);
  system(cmd.c_str());
  usleep(1000);
  if (trigger_type == "aux") {
    cmd = StringUtil::form("trigft -%d %s %d -1", ftswid,
                           trigger_type.c_str(), trigger_limit);
  } else {
    cmd = StringUtil::form("trigft -%d %s %d %d", ftswid,
                           trigger_type.c_str(), dummy_rate, trigger_limit);
  }
  LogFile::debug(cmd);
  system(cmd.c_str());
  monitor();
  /*
  obj.addInt("expno", (int)g_ftstat.exp);
  obj.addInt("runno", (int)g_ftstat.run);
  obj.addInt("subno", (int)g_ftstat.sub);
  obj.addInt("tincnt", g_ftstat.tincnt);
  obj.addInt("toutcnt", g_ftstat.toutcnt);
  obj.addInt("tlimit", (int)g_ftstat.tlimit);
  obj.addInt("tlast", (int)g_ftstat.tlast);
  obj.addText("err", std::string(g_ftstat.err));
  obj.addText("errport", std::string(g_ftstat.errport));
  obj.addInt("tstart", (int)g_ftstat.tstart);
  obj.addText("tstart_s", Date(g_ftstat.tstart).toString());
  obj.addInt("trun", (int)g_ftstat.trun);
  obj.addText("ftstate", g_ftstat.state);
  */
}

void TTDACallback::stop() throw(RCHandlerException)
{
  DBObject& obj(getDBObject());
  int ftswid = obj.getInt("ftsw");
  std::string cmd = StringUtil::form("trigft -%d reset", ftswid);
  LogFile::debug(cmd);
  system(cmd.c_str());
  obj.addInt("expno", (int)g_ftstat.exp);
  obj.addInt("runno", (int)g_ftstat.run);
  obj.addInt("subno", (int)g_ftstat.sub);
  obj.addInt("tincnt", g_ftstat.tincnt);
  obj.addInt("toutcnt", g_ftstat.toutcnt);
  obj.addInt("tlimit", (int)g_ftstat.tlimit);
  obj.addInt("tlast", (int)g_ftstat.tlast);
  obj.addText("err", std::string(g_ftstat.err));
  obj.addText("errport", std::string(g_ftstat.errport));
  obj.addInt("tstart", (int)g_ftstat.tstart);
  obj.addText("tstart_s", Date(g_ftstat.tstart).toString());
  obj.addInt("trun", (int)g_ftstat.trun);
  obj.addText("ftstate", g_ftstat.state);
}

bool TTDACallback::pause() throw(RCHandlerException)
{
  if (m_ttdnode.getName().size() > 0) {
    send(NSMMessage(m_ttdnode, NSMCommand(14, "PAUSE")));
  }
  return true;
}

bool TTDACallback::resume(int /*subno*/) throw(RCHandlerException)
{
  if (m_ttdnode.getName().size() > 0) {
    send(NSMMessage(m_ttdnode, NSMCommand(15, "RESUME")));
  }
  return true;
}

void TTDACallback::recover(const DBObject&) throw(RCHandlerException)
{
  if (m_ttdnode.getName().size() > 0) {
    send(NSMMessage(m_ttdnode, NSMCommand(16, "RECOVER")));
  }
}

void TTDACallback::abort() throw(RCHandlerException)
{
  stop();
  setState(RCState::NOTREADY_S);
}

void TTDACallback::trigft() throw(RCHandlerException)
{
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
    std::string cmd = StringUtil::form("trigft -%d reset", ftswid);
    LogFile::debug(cmd);
    system(cmd.c_str());
  } catch (const std::out_of_range& e) {
    LogFile::error(e.what());
  }
}

void TTDACallback::trigio(const std::string& type) throw(RCHandlerException)
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

void TTDACallback::send(const NSMMessage& msg) throw(RCHandlerException)
{
  try {
    if (NSMCommunicator::send(msg)) {
      return;
    }
  } catch (const NSMHandlerException& e) {
  } catch (const TimeoutException& e) {

  }
  throw (RCHandlerException("Failed to send %s to %s",
                            msg.getRequestName(), msg.getNodeName()));
}

void TTDACallback::ok(const char* nodename, const char* data) throw()
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

void TTDACallback::error(const char* nodename, const char* data) throw()
{
  if (m_ttdnode.getName().size() > 0 &&
      m_ttdnode.getName() == nodename) {
    LogFile::debug("ERROR from %s : %s", nodename, data);
  }
}

void TTDACallback::resetft() throw()
{
  DBObject& obj(getDBObject());
  get(obj);
  int ftswid = obj.getInt("ftsw");
  std::string cmd = StringUtil::form("resetft -%d", ftswid);
  LogFile::info(cmd);
  g_flag = false;
  system(cmd.c_str());
}
