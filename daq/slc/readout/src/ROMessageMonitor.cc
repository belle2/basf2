#include "daq/slc/readout/ROMessageMonitor.h"

#include "daq/slc/readout/ROController.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/base/SystemLog.h"
#include "daq/slc/base/StringUtil.h"
#include <daq/slc/base/Debugger.h>

using namespace Belle2;

void ROMessageMonitor::run()
{
  while (true) {
    int priority = 0;
    std::string message = _con->getLog().recieve(priority);
    _con->lock();
    if (priority == RunLogMessanger::NOTICE) {
      State state = State(message);
      if (state != State::UNKNOWN) {
        _con->setState(state);
        message = "";
        _con->signal();
      }
    } else if (priority == RunLogMessanger::ERROR) {
      _con->setState(State::ERROR_ES);
      _con->signal();
    } else if (priority == RunLogMessanger::FATAL) {
      _con->setState(State::FATAL_ES);
      _con->signal();
    }
    _con->setMessage(message);
    _con->unlock();
    if (message.size() > 0) {
      Belle2::debug("priority = %d, message = %s", priority, message.c_str());
      SystemLog::Priority sys_priority = SystemLog::DEBUG;
      if (priority == RunLogMessanger::INFO) sys_priority = SystemLog::INFO;
      else if (priority == RunLogMessanger::NOTICE) sys_priority = SystemLog::NOTICE;
      else if (priority == RunLogMessanger::WARNING) sys_priority = SystemLog::WARNING;
      else if (priority == RunLogMessanger::ERROR) sys_priority = SystemLog::ERROR;
      else if (priority == RunLogMessanger::FATAL) sys_priority = SystemLog::FATAL;
      NSMCommunicator* comm = _con->getCallback()->getCommunicator();
      NSMNode* node = _con->getCallback()->getNode();
      try {
        comm->sendLog(SystemLog(node->getName(), sys_priority, message));
      } catch (const NSMHandlerException& e) {

      }
    }
  }
}
