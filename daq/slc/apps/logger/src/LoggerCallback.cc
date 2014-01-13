#include "daq/slc/apps/logger/LoggerCallback.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/SystemLog.h"

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
  SystemLog log;
  NSMMessage& msg(getMessage());
  log.unpack(msg.getNParams(), (const int*)msg.getParams(),
             msg.getData());
  log.setGroupName(_comm->getNode()->getName());
  try {
    _comm->sendLog(log);
  } catch (const NSMHandlerException& e) {
    LogFile::fatal("Failed to send log : %s", e.what());
    return false;
  }
  LogFile::debug(log.toString());
  return true;
}

