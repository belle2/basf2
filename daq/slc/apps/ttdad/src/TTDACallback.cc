#include "daq/slc/apps/ttdad/TTDACallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>

namespace Belle2 {

  class NSMVHandlerTrigft : public NSMVHandlerInt {
  public:
    NSMVHandlerTrigft(TTDACallback& callback, const std::string& name)
      : NSMVHandlerInt(name, false, true), m_callback(callback) {}
    virtual ~NSMVHandlerTrigft() throw() {}
    bool handleInt(int val) {
      if (val > 0) {
        m_callback.trigft();
      }
      return true;
    }
  private:
    TTDACallback& m_callback;
  };

}

using namespace Belle2;

bool TTDACallback::initialize(const DBObject& obj) throw()
{
  return configure(obj);
}

bool TTDACallback::configure(const DBObject&) throw()
{
  add(new NSMVHandlerTrigft(*this, "trgft"));
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
    int pars[3];
    get("trigger_type", pars[0]);
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

