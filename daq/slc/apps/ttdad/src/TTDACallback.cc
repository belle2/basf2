#include "daq/slc/apps/ttdad/TTDACallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

namespace Belle2 {

  class NSMVHandlerTrigft : public NSMVHandlerInt {
  public:
    NSMVHandlerTrigft(TTDACallback& callback, const std::string& name)
      : NSMVHandlerInt(name, false, true), m_callback(callback) {}
    virtual ~NSMVHandlerTrigft() throw() {}
    bool handleSetInt(int val) {
      LogFile::debug("trigft");
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
    bool handleSetText(const std::string& val) {
      return m_callback.setTriggerType(val);
    }
  private:
    TTDACallback& m_callback;
  };

}

using namespace Belle2;

TTDACallback::TTDACallback(const RCNode& ttd)
  : RCCallback(5), m_ttdnode(ttd)
{
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

bool TTDACallback::initialize(const DBObject& obj) throw()
{
  return configure(obj);
}

bool TTDACallback::configure(const DBObject& obj) throw()
{
  add(new NSMVHandlerTrigft(*this, "trigft"));
  add(new NSMVHandlerTriggerType(*this, "trigger_type", obj.getText("trigger_type")));
  return true;
}

void TTDACallback::timeout(NSMCommunicator&) throw()
{
}

void TTDACallback::load(const DBObject&) throw(RCHandlerException)
{
  trigft();
}

void TTDACallback::start(int expno, int runno) throw(RCHandlerException)
{
  int pars[2] = { expno, runno };
  send(NSMMessage(m_ttdnode, NSMCommand(12, "START"), 2, pars));
}

void TTDACallback::stop() throw(RCHandlerException)
{
  send(NSMMessage(m_ttdnode, NSMCommand(13, "STOP")));
}

void TTDACallback::pause() throw(RCHandlerException)
{
  send(NSMMessage(m_ttdnode, NSMCommand(14, "PAUSE")));
}

void TTDACallback::resume() throw(RCHandlerException)
{
  send(NSMMessage(m_ttdnode, NSMCommand(15, "RESUME")));
}

void TTDACallback::recover() throw(RCHandlerException)
{
  send(NSMMessage(m_ttdnode, NSMCommand(16, "RECOVER")));
}

void TTDACallback::abort() throw(RCHandlerException)
{
  send(NSMMessage(m_ttdnode, NSMCommand(13, "STOP")));
}

void TTDACallback::trigft() throw(RCHandlerException)
{
  try {
    LogFile::debug("trigft");
    int pars[3];
    get("trigger_type", m_trigger_type);
    pars[0] = m_trgcommands[m_trigger_type];
    get("dummy_rate", pars[1]);
    get("trigger_limit", pars[2]);
    send(NSMMessage(m_ttdnode, NSMCommand(11, "LOAD"), 3, pars));
  } catch (const std::out_of_range& e) {
  }
}

void TTDACallback::send(const NSMMessage& msg) throw(RCHandlerException)
{
  try {
    if (NSMCommunicator::send(msg)) return;
  } catch (const NSMHandlerException& e) {}
  throw (RCHandlerException("Failed to send %s to %s",
                            msg.getRequestName(), msg.getNodeName()));
}

