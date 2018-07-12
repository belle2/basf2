#include "daq/slc/apps/arichtempd/ArichtempCallback.h"

#include <daq/slc/system/LogFile.h>

#include <stdlib.h>

namespace Belle2 {
  class ArichtempVHandler : public NSMVHandlerText {
  public:
    ArichtempVHandler(const std::string& name, const std::string& val)
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

ArichtempCallback::ArichtempCallback(const std::string& name, int timeout)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)", name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);
}

ArichtempCallback::~ArichtempCallback() throw()
{
}

void ArichtempCallback::init(NSMCommunicator&) throw()
{
  add(new NSMVHandlerFloat("ival", true, false, 10));
  add(new NSMVHandlerFloat("fval", true, true, 0.1));
  add(new ArichtempVHandler("tval", "example"));
}

void ArichtempCallback::timeout(NSMCommunicator&) throw()
{
  int ival = rand() % 256;
  set("ival", ival);
  LogFile::debug("ival updated: %d", ival);
}

