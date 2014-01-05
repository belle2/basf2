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
    SystemLog::Priority sys_priority = (SystemLog::Priority)priority;
    if (sys_priority > SystemLog::UNKNOWN && message.size() > 0) {
      std::string level_s = "DEBUG";
      if (sys_priority == SystemLog::INFO) level_s = "INFO";
      else if (sys_priority == SystemLog::NOTICE) level_s = "NOTICE";
      else if (sys_priority == SystemLog::WARNING) level_s = "WARNING";
      else if (sys_priority == SystemLog::ERROR) level_s = "ERROR";
      else if (sys_priority == SystemLog::FATAL) level_s = "FATAL";
      Belle2::debug("[%s] %s", level_s.c_str(), message.c_str());
      _con->lock();
      NSMCommunicator* comm = _con->getCallback()->getCommunicator();
      NSMNode* node = _con->getCallback()->getNode();
      try {
        if (sys_priority == SystemLog::ERROR) {
          node->setState(State::ERROR_ES);
          comm->sendError(message);
        }
        if (sys_priority == SystemLog::FATAL) {
          node->setState(State::FATAL_ES);
          comm->sendError(message);
        }
        comm->sendLog(SystemLog(node->getName(), sys_priority, message));
      } catch (const NSMHandlerException& e) {

      }
      _con->unlock();
    }
  }
}
