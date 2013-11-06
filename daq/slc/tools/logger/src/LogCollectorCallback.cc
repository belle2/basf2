#include "LogCollectorCallback.h"

#include "GUICommunicator.h"

#include "base/LogMessage.h"

#include "nsm/NSMCommunicator.h"

#include <iostream>

using namespace Belle2;

LogCollectorCallback::LogCollectorCallback(NSMNode* node)
  : LogCallback(node)
{
}

LogCollectorCallback::~LogCollectorCallback() throw()
{
}

bool LogCollectorCallback::log() throw()
{
  LogMessage log;
  NSMMessage& msg(getMessage());
  log.unpack(msg.getNParams(), (const int*)msg.getParams(),
             msg.getData());
  GUICommunicator::push(log);
  std::cout << log.toString() << std::endl;
  return true;
}

