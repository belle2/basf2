#include "daq/slc/apps/logtestd/LogtestCallback.h"

#include <daq/slc/system/LogFile.h>

#include <stdlib.h>

namespace Belle2 {
  class LogtestVHandler : public NSMVHandlerText {
  public:
    LogtestVHandler(const std::string& name, const std::string& val)
      : NSMVHandlerText(name, true, true, val) {}
    bool handleGetText(std::string& val)
    {
      NSMVHandlerText::handleGetText(val);
      LogFile::info("%s is read : %s", getName().c_str(), val.c_str());
      return true;
    }
    bool handleSetText(const std::string& val)
    {
      LogFile::info("%s is written : %s", getName().c_str(), val.c_str());
      return NSMVHandlerText::handleSetText(val);
    }
  };
}

using namespace Belle2;

LogtestCallback::LogtestCallback(const std::string& name, int timeout)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)", name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);
}

LogtestCallback::~LogtestCallback() throw()
{
}

void LogtestCallback::init(NSMCommunicator&) throw()
{
  add(new NSMVHandlerInt("ival", true, false, 10));
  add(new NSMVHandlerFloat("fval", true, true, 0.1));
  add(new LogtestVHandler("tval", "example"));
}

void LogtestCallback::timeout(NSMCommunicator&) throw()
{
  static int count = 1;
  log(LogFile::INFO, "test log %d", ++count);
}

