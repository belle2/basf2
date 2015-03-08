#include "NSM2CACallback.h"

#include <daq/slc/runcontrol/RCState.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <recGbl.h>
#include <dbAccess.h>

using namespace Belle2;

bool NSMVHandler2CA::handleSet(const NSMVar& var)
{
  NSMVHandler::handleSet(var);
  scanIoRequest(m_pvt);
  return true;
}

NSM2CACallback* NSM2CACallback::g_callback = NULL;

NSM2CACallback& NSM2CACallback::get() {
  if (!g_callback) g_callback = new NSM2CACallback();
  return *g_callback;
}

void NSM2CACallback::init(NSMCommunicator&) throw()
{
}

void NSM2CACallback::timeout(NSMCommunicator&) throw()
{
  NSMVHandlerList& handlers(getHandlers());
  static unsigned long long count = 0;
  for (NSMVHandlerList::iterator it = handlers.begin();
       it != handlers.end(); it++ ) {
    NSMVHandler& handler(*(*it));
    std::string nodename = handler.getNode();
    if (handler.getName() == "rcstate" && nodename.size() > 0 ) {
      std::string s = handler.get().getText();
      try {
	RCState state(s);
	NSMCommunicator::connected(nodename);
	if (state == RCState::UNKNOWN) {
	  get(NSMNode(nodename), "rcstate", s);
	  LogFile::info("%s got up (%s)", nodename.c_str(), s.c_str());
 	}
      } catch (const NSMNotConnectedException& e) {
	if (s != "UNKNOWN") {
	  LogFile::warning("%s got down", nodename.c_str());
	  handler.handleSet(NSMVar(nodename, std::string("UNKNOWN")));
	}
      } catch (const TimeoutException& e) {
	LogFile::warning(e.what());
      } 
    } else if (count % 10 == 0){
      NSMCommunicator::send(NSMMessage(NSMNode(nodename), NSMCommand::VGET, handler.getName()));
    }
  }
  count++;
}

void NSM2CACallback::error(const char* nodename, const char* data) throw()
{
  LogFile::error("Error from %s : %s", nodename, data);
  try {
    getHandler(nodename, "rcstate").handleSet(NSMVar("rcstate", "NOTREADY"));
  } catch (const std::out_of_range& e) {
    LogFile::error(e.what());
  }
}

void NSM2CACallback::ok(const char* nodename, const char* data) throw()
{
  RCState state(data);
  if (state != RCState::UNKNOWN) {
    LogFile::debug("ok from %s (%s)", nodename, data);
    std::string s;
    try {
      getHandler(nodename, "rcstate").handleSet(NSMVar("rcstate", data));
    } catch (const std::out_of_range& e) {
      LogFile::error(e.what());
    }
  }
}
