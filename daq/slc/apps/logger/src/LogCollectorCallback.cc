#include "daq/slc/apps/logger/LogCollectorCallback.h"

#include "daq/slc/apps/logger/LogUICommunicator.h"
#include "daq/slc/apps/logger/LogDBManager.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/SystemLog.h"
#include "daq/slc/base/Debugger.h"

using namespace Belle2;

LogCollectorCallback::LogCollectorCallback(NSMNode* node,
                                           LogDBManager* man)
  : LogCallback(node), _man(man)
{
  if (_man != NULL) {
    try {
      _man->createTable();
    } catch (const DBHandlerException& e) {
      LogFile::debug(e.what());
    }
  }
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
  if (_man != NULL) {
    try {
      _man->writeLog(log);
    } catch (const DBHandlerException& e) {
      LogFile::fatal("Failed to write new log : %s", e.what());
    }
  }
  LogFile::debug(log.toString());
  return true;
}

