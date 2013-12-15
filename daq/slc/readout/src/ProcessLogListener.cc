#include "daq/slc/readout/ProcessLogListener.h"

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/base/SystemLog.h"
#include "daq/slc/base/StringUtil.h"
#include <daq/slc/base/Debugger.h>

using namespace Belle2;

void ProcessLogListener::run()
{
  while (true) {
    SystemLog::Priority priority = SystemLog::UNKNOWN;
    std::string message = _con->getLog().recieve(priority);
    _con->lock();
    if (priority == SystemLog::NOTICE) {
      State state = State(message);
      if (state != State::UNKNOWN) {
        _con->setState(state);
        message = "";
        _con->signal();
      }
    } else if (priority == SystemLog::ERROR) {
      _con->setState(State::ERROR_ES);
      _con->signal();
    } else if (priority == SystemLog::FATAL) {
      _con->setState(State::FATAL_ES);
      _con->signal();
    }
    _con->setMessage(message);
    _con->unlock();
    if (message.size() > 0) {
      Belle2::debug("priority = %d, message = %s", priority, message.c_str());
      SystemLog::Priority sys_priority = SystemLog::DEBUG;
      if (priority == SystemLog::INFO) sys_priority = SystemLog::INFO;
      else if (priority == SystemLog::NOTICE) sys_priority = SystemLog::NOTICE;
      else if (priority == SystemLog::WARNING) sys_priority = SystemLog::WARNING;
      else if (priority == SystemLog::ERROR) sys_priority = SystemLog::ERROR;
      else if (priority == SystemLog::FATAL) sys_priority = SystemLog::FATAL;
      NSMCommunicator* comm = _con->getCallback()->getCommunicator();
      NSMNode* node = _con->getCallback()->getNode();
      try {
        comm->sendLog(SystemLog(node->getName(), sys_priority, message));
      } catch (const NSMHandlerException& e) {

      }
    }
  }
}
