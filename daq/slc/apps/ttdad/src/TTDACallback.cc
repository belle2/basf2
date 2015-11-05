#include "daq/slc/apps/ttdad/TTDACallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

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

  class NSMVHandlerTriggerType : public NSMVHandlerText {
  public:
    NSMVHandlerTriggerType(TTDACallback& callback,
                           const std::string& name, const std::string& type)
      : NSMVHandlerText(name, true, true, type), m_callback(callback) {}
    virtual ~NSMVHandlerTriggerType() throw() {}
    bool handleSetText(const std::string& val)
    {
      return m_callback.setTriggerType(val);
    }
  private:
    TTDACallback& m_callback;
  };

}

using namespace Belle2;

TTDACallback::TTDACallback(const RCNode& ttd)
  : RCCallback(1), m_ttdnode(ttd)
{
  if (m_ttdnode.getName().size() > 0) {
    setAutoReply(false);
    m_ttdnode.setState(Enum::UNKNOWN);
  }
  m_trgcommands.insert(std::map<std::string, int>::value_type("none", 0));
  m_trgcommands.insert(std::map<std::string, int>::value_type("aux", 1));
  m_trgcommands.insert(std::map<std::string, int>::value_type("i", 2));
  m_trgcommands.insert(std::map<std::string, int>::value_type("tlu", 3));
  m_trgcommands.insert(std::map<std::string, int>::value_type("pulse", 4));
  m_trgcommands.insert(std::map<std::string, int>::value_type("revo", 5));
  m_trgcommands.insert(std::map<std::string, int>::value_type("random", 6));
  m_trgcommands.insert(std::map<std::string, int>::value_type("possion", 7));
  m_trgcommands.insert(std::map<std::string, int>::value_type("once", 8));
  m_trgcommands.insert(std::map<std::string, int>::value_type("stop", 9));
}

bool TTDACallback::setTriggerType(const std::string& type) throw()
{
  if (m_trgcommands.find(type) != m_trgcommands.end()) {
    m_trigger_type = type;
    return true;
  }
  return false;
}

void TTDACallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  configure(obj);
}

void TTDACallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  add(new NSMVHandlerInt("ftsw", true, false, m_ftswid));
  add(new NSMVHandlerTrigft(*this, "trigft"));
  add(new NSMVHandlerTriggerType(*this, "trigger_type", obj.getText("trigger_type")));
  try {
    trigft();
  } catch (const RCHandlerException& e) {
    LogFile::warning(e.what());
  }
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
  }
}

void TTDACallback::load(const DBObject&) throw(RCHandlerException)
{
  trigft();
}

void TTDACallback::start(int expno, int runno) throw(RCHandlerException)
{
  if (m_ttdnode.getName().size() > 0) {
    int pars[2] = { expno, runno };
    send(NSMMessage(m_ttdnode, NSMCommand(12, "START"), 2, pars));
  } else {
    std::string cmd = StringUtil::form("/home/usr/nakao/bin/regft -%d 160 0x%x", m_ftswid, runno << 8);
    LogFile::debug(cmd);
    system(cmd.c_str());
    sleep(1);
    trigft();
    cmd = StringUtil::form("/home/usr/nakao/bin/trigft -%d %s", m_ftswid, m_trigger_type.c_str());
    LogFile::debug(cmd);
    system(cmd.c_str());
  }
}

void TTDACallback::stop() throw(RCHandlerException)
{
  if (m_ttdnode.getName().size() > 0) {
    send(NSMMessage(m_ttdnode, NSMCommand(13, "STOP")));
  } else {
    std::string cmd = StringUtil::form("/home/usr/nakao/bin/trigft -%d reset", m_ftswid);
    LogFile::debug(cmd);
    system(cmd.c_str());
  }
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
  /*
  if (m_ttdnode.getName().size() > 0) {
    send(NSMMessage(m_ttdnode, NSMCommand(13, "STOP")));
  }
  */
}

void TTDACallback::trigft() throw(RCHandlerException)
{
  try {
    int pars[3];
    get("trigger_type", m_trigger_type);
    pars[0] = m_trgcommands[m_trigger_type];
    get("dummy_rate", pars[1]);
    get("trigger_limit", pars[2]);
    if (m_ttdnode.getName().size() > 0) {
      send(NSMMessage(m_ttdnode, NSMCommand(11, "TRIGFT"), 3, pars));
    } else {
      std::string cmd = StringUtil::form("/home/usr/nakao/bin/utimeft -%d", m_ftswid);
      LogFile::debug(cmd);
      system(cmd.c_str());
      sleep(1);
      cmd = StringUtil::form("/home/usr/nakao/bin/trigft -%d reset", m_ftswid);
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
      setState(RCState::NOTREADY_S);
    } catch (const std::out_of_range& e) {}
  }
}
