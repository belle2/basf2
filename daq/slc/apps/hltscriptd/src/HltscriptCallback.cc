#include "daq/slc/apps/hltscriptd/HltscriptCallback.h"

#include <daq/slc/system/LogFile.h>

#include <stdlib.h>
#include <unistd.h>

namespace Belle2 {

  class HltscriptVHandler : public NSMVHandlerText {
  public:
    HltscriptVHandler(NSMCallback callback, const std::string& name, const std::string& val)
      : NSMVHandlerText(name, true, true, val), m_callback(callback) {}
    bool handleSetText(const std::string& script)
    {
      //m_callback.log(LogFile::INFO, "HLT script switched to "+script);
      LogFile::info("HLT script switched to " + script);
      const char* linkpath = "/user/b2daq/TestBeam_Feb17/HLT/vxdroi/run/evp_scripts/script.py";
      unlink(linkpath);
      symlink(("/user/b2daq/TestBeam_Feb17/HLT/belle2/releases/DESY_testbeam_Feb17/testbeam/daq/hlt/evp_scripts/" + script).c_str(),
              linkpath);
      return NSMVHandlerText::handleSetText(script);
    }

  private:
    NSMCallback& m_callback;
  };

}

using namespace Belle2;

HltscriptCallback::HltscriptCallback(const std::string& name, int timeout)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)", name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);
}

HltscriptCallback::~HltscriptCallback() throw()
{
}

void HltscriptCallback::init(NSMCommunicator&) throw()
{
  add(new HltscriptVHandler(*this, "script", "none"));
}

void HltscriptCallback::timeout(NSMCommunicator&) throw()
{
}

