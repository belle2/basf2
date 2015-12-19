#include "daq/slc/apps/logreaderd/LogreaderCallback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <stdlib.h>

using namespace Belle2;

LogreaderCallback::LogreaderCallback(const std::string& name, int timeout)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)",
                 name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);
}

LogreaderCallback::~LogreaderCallback() throw()
{
}

void LogreaderCallback::init(NSMCommunicator&) throw()
{
}

void LogreaderCallback::timeout(NSMCommunicator&) throw()
{
  try {
    if (getLogNode().getName().size() > 0) {
      NSMCommunicator::send(NSMMessage(getLogNode().getName(),
                                       NSMCommand::LOGGET));
    }
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
}

void LogreaderCallback::logset(const DAQLogMessage& msg) throw()
{
  LogFile::put(msg.getPriority(), "%s :%s ",
               msg.getNodeName().c_str(), msg.getMessage().c_str());
}
