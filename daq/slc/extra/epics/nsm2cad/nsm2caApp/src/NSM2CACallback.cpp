#include "NSM2CACallback.h"

#include <daq/slc/runcontrol/RCState.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <recGbl.h>
#include <dbAccess.h>

using namespace Belle2;

bool NSMVHandler2CA::handleSet(const NSMVar& var)
{
  LogFile::info("%s.%s", var.getNode().c_str(), var.getName().c_str());
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
  if (count % 20 == 0){
    for (NSMVHandlerList::iterator it = handlers.begin();
	 it != handlers.end(); it++ ) {
      NSMVHandler& handler(*(*it));
      std::string nodename = handler.getNode();
      try {
	NSMNode node(nodename);
	NSMCommunicator::send(NSMMessage(node, NSMCommand::VGET, handler.getName()));
	perform(wait(node, NSMCommand::VSET, 5));
      } catch (const IOException& e) {
	LogFile::warning(e.what());
      }
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
      NSMVar var("rcstate", data);
      var.setNode(nodename);
      getHandler(nodename, "rcstate").handleSet(var);
    } catch (const std::out_of_range& e) {
      LogFile::error(e.what());
    }
  }
}
