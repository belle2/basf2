#include "LoggerCallback.h"

#include "base/LogMessage.h"

#include "nsm/NSMCommunicator.h"

#include <iostream>

using namespace Belle2;

LoggerCallback::LoggerCallback(NSMNode* node, NSMCommunicator* comm)
  : LogCallback(node), _comm(comm)
{
}

LoggerCallback::~LoggerCallback() throw()
{
}

bool LoggerCallback::log() throw()
{
  LogMessage log;
  NSMMessage& msg(getMessage());
  log.unpack(msg.getNParams(), (const int*)msg.getParams(),
             msg.getData());
  log.setGroupName(_comm->getNode()->getName());
  std::cout << log.toString() << std::endl;
  try {
    _comm->sendLog(log);
  } catch (const NSMHandlerException& e) {
    std::cout << "Failed to send log" << std::endl;
    return false;
  }
  return true;
}

