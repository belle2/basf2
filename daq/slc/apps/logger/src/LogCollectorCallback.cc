#include "daq/slc/apps/logger/LogCollectorCallback.h"

#include "daq/slc/apps/logger/LogUICommunicator.h"

#include "daq/slc/base/SystemLog.h"

#include "daq/slc/nsm/NSMCommunicator.h"

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
  SystemLog log;
  NSMMessage& msg(getMessage());
  log.unpack(msg.getNParams(),
             (const int*)msg.getParams(), msg.getData());
  if (log.getGroupName().size() == 0) {
    log.setGroupName("GLOBAL");
  }
  LogUICommunicator::push(log);
  std::cout << log.toString() << std::endl;
  return true;
}

