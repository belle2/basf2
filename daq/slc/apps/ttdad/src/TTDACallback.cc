#include "daq/slc/apps/ttdad/TTDACallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>

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
}

unsigned int prev_u = 0;
unsigned int prev_c = 0;

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

}

using namespace Belle2;

TTDACallback::TTDACallback() : RCCallback(4)
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
  configure(obj);
  openData(m_ttdnode.getName() + "FAST", "pocket_ttd_fast", pocket_ttd_fast_revision);
  openData(m_ttdnode.getName() + "SLOW", "pocket_ttd", pocket_ttd_revision);
}

void TTDACallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  if (obj.hasText("ttd")) {
    std::string ttd = obj.getText("ttd");
    m_ttdnode = RCNode(ttd);
    setAutoReply(false);
  } else {
    setAutoReply(true);
  }
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
}

void TTDACallback::monitor() throw(RCHandlerException)
{
  if (m_ttdnode.getName().size() > 0) {
    RCState cstate(m_ttdnode.getState());
    try {
      NSMCommunicator::connected(m_ttdnode.getName());
      if (cstate == Enum::UNKNOWN) {
        NSMCommunicator::send(NSMMessage(m_ttdnode, NSMCommand(17, "STATECHECK")));
      }
    } catch (const NSMNotConnectedException&) {
      if (cstate != Enum::UNKNOWN) {
        m_ttdnode.setState(Enum::UNKNOWN);
        setState(RCState::NOTREADY_S);
      }
    } catch (const NSMHandlerException& e) {
      LogFile::error(e.what());
    }
    NSMData& data_fast(getData(m_ttdnode.getName() + "FAST"));
    NSMData& data_slow(getData(m_ttdnode.getName() + "SLOW"));
    try {
      if (data_fast.isAvailable() && data_slow.isAvailable()) {
        fast_t* f = (fast_t*)data_fast.get();
        slow_t* s = (slow_t*)data_slow.get();
        struct timeval tv;
        gettimeofday(&tv, 0);
        struct tm* tp = localtime(&tv.tv_sec);
        if (prev_u != f->utime || prev_c != f->ctime) {
          sprintf(g_ftstat.statft,
                  "statft version %d FTSW(NSM) - "
                  "%04d.%02d.%02d %02d:%02d:%02d.%03d\n",
                  VERSION, tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday,
                  tp->tm_hour, tp->tm_min, tp->tm_sec, (int)(tv.tv_usec / 1000));
          color2u067(f, s);
          summary2u067(&tv, f, s);
          set("expno", (int)g_ftstat.exp);
          set("runno", (int)g_ftstat.run);
          set("subno", (int)g_ftstat.sub);
          set("busy", g_ftstat.busy);
          set("reset", g_ftstat.reset);
          set("stafifo", g_ftstat.stafifo);
          set("tincnt", (float)g_ftstat.tincnt);
          set("toutcnt", (float)g_ftstat.toutcnt);
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
        }
        prev_u = f->utime;
        prev_c = f->ctime;
      }
    } catch (const NSMHandlerException& e) {}
  }
}

void TTDACallback::load(const DBObject&) throw(RCHandlerException)
{
  resetft();
  trigft();
  send(NSMMessage(m_ttdnode, NSMCommand(13, "LOAD")));
  setState(RCState::READY_S);
}

void TTDACallback::start(int expno, int runno) throw(RCHandlerException)
{
  DBObject& obj(getDBObject());
  get(obj);
  int ftswid = obj.getInt("ftsw");
  int dummy_rate = obj.getInt("dummy_rate");
  int trigger_limit = obj.getInt("trigger_limit");
  std::string trigger_type = obj.getText("trigger_type");
  if (m_ttdnode.getName().size() > 0) {
    int pars[3];
    pars[0] = m_trgcommands[trigger_type];
    pars[1] = dummy_rate * 1000;
    pars[2] = trigger_limit;
    send(NSMMessage(m_ttdnode, NSMCommand(11, "TRIGFT"), 3, pars));
    pars[0] = expno;
    pars[1] = runno;
    send(NSMMessage(m_ttdnode, NSMCommand(12, "START"), 2, pars));
  } else {
    std::string cmd = StringUtil::form("regft -%d 160 0x%x", ftswid, (expno << 22) + ((runno - 1) << 8));
    LogFile::debug(cmd);
    system(cmd.c_str());
    sleep(1);
    trigft();
    cmd = StringUtil::form("trigft -%d %s %d %d", ftswid,
                           trigger_type.c_str(), dummy_rate, trigger_limit);
    LogFile::debug(cmd);
    system(cmd.c_str());
  }
  monitor();
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

void TTDACallback::stop() throw(RCHandlerException)
{
  DBObject& obj(getDBObject());
  int ftswid = obj.getInt("ftsw");
  if (m_ttdnode.getName().size() > 0) {
    send(NSMMessage(m_ttdnode, NSMCommand(13, "STOP")));
  } else {
    std::string cmd = StringUtil::form("trigft -%d reset", ftswid);
    LogFile::debug(cmd);
    system(cmd.c_str());
  }
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
    int pars[3];
    pars[0] = m_trgcommands[trigger_type];
    pars[1] *= 1000;
    pars[2] = trigger_limit;
    pars[3] = dummy_rate;
    if (m_ttdnode.getName().size() > 0) {
      send(NSMMessage(m_ttdnode, NSMCommand(11, "TRIGFT"), 3, pars));
    } else {
      std::string cmd = StringUtil::form("trigft -%d reset", ftswid);
      LogFile::debug(cmd);
      system(cmd.c_str());
      sleep(1);
    }
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
    try {
      //setState(RCState::NOTREADY_S);
    } catch (const std::out_of_range& e) {}
  }
}

void TTDACallback::resetft() throw()
{
  if (m_ttdnode.getName().size() > 0) {
    LogFile::info("resetft");
    send(NSMMessage(m_ttdnode, NSMCommand(17, "RESETFT")));
  } else {
    DBObject& obj(getDBObject());
    get(obj);
    int ftswid = obj.getInt("ftsw");
    std::string cmd = StringUtil::form("resetft -%d", ftswid);
    LogFile::info(cmd);
    system(cmd.c_str());
  }
}
