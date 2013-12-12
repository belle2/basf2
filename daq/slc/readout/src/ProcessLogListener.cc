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
    int priority = 0;
    std::string message = _con->getLog().recieve(priority);
    _con->lock();
    if (priority == ProcessLogBuffer::NOTICE) {
      State state = State(message);
      if (state != State::UNKNOWN) {
        _con->setState(state);
        message = "";
        _con->signal();
      }
    } else if (priority == ProcessLogBuffer::ERROR) {
      _con->setState(State::ERROR_ES);
      _con->signal();
    } else if (priority == ProcessLogBuffer::FATAL) {
      _con->setState(State::FATAL_ES);
      _con->signal();
    }
    _con->setMessage(message);
    _con->unlock();
    if (message.size() > 0) {
      Belle2::debug("priority = %d, message = %s", priority, message.c_str());
      SystemLog::Priority sys_priority = SystemLog::DEBUG;
      if (priority == ProcessLogBuffer::INFO) sys_priority = SystemLog::INFO;
      else if (priority == ProcessLogBuffer::NOTICE) sys_priority = SystemLog::NOTICE;
      else if (priority == ProcessLogBuffer::WARNING) sys_priority = SystemLog::WARNING;
      else if (priority == ProcessLogBuffer::ERROR) sys_priority = SystemLog::ERROR;
      else if (priority == ProcessLogBuffer::FATAL) sys_priority = SystemLog::FATAL;
      NSMCommunicator* comm = _con->getCallback()->getCommunicator();
      NSMNode* node = _con->getCallback()->getNode();
      try {
        comm->sendLog(SystemLog(node->getName(), sys_priority, message));
      } catch (const NSMHandlerException& e) {

      }
    }
  }
}
