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
      SystemLog::Priority sys_priority = (SystemLog::Priority)priority;
      std::string level_s = "DEBUG";
      if (sys_priority == SystemLog::INFO) level_s = "INFO";
      else if (sys_priority == SystemLog::NOTICE) level_s = "NOTICE";
      else if (sys_priority == SystemLog::WARNING) level_s = "WARNING";
      else if (sys_priority == SystemLog::ERROR) level_s = "ERROR";
      else if (sys_priority == SystemLog::FATAL) level_s = "FATAL";
      Belle2::debug("[%s] %s", level_s.c_str(), message.c_str());
      NSMCommunicator* comm = _con->getCallback()->getCommunicator();
      NSMNode* node = _con->getCallback()->getNode();
      try {
        comm->sendLog(SystemLog(node->getName(), sys_priority, message));
      } catch (const NSMHandlerException& e) {

      }
    }
  }
}
