#include "NSM2CACallback.h"
#include "NSM2CA.h"

#include <daq/slc/runcontrol/RCState.h>
#include <daq/slc/hvcontrol/HVState.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <recGbl.h>
#include <dbAccess.h>
#include <aiRecord.h>
#include <longinRecord.h>
#include <stringinRecord.h>

using namespace Belle2;

NSM2CACallback* NSM2CACallback::g_callback = NULL;

NSM2CACallback& NSM2CACallback::get() {
  if (!g_callback) g_callback = new NSM2CACallback();
  return *g_callback;
}

void NSM2CACallback::init(NSMCommunicator& com) throw()
{
}

void NSM2CACallback::timeout(NSMCommunicator&) throw()
{
  static unsigned long long count = 0;
  if (count % 20 == 2){
    m_nsm2ca->lock();
    NSMVarPVTList& vars(m_nsm2ca->getVars());
    for (NSMVarPVTList::iterator it = vars.begin();
	 it != vars.end(); it++ ) {
      NSMVar& var((it->second).var);
      std::string vname = var.getName();
      if (vname == "rcrequest" || vname == "hvrequest") continue;
      std::string node = var.getNode();
      try {
	NSMMessage msg(NSMNode(node), NSMCommand::VGET, vname);
	NSMCommunicator::send(msg);
      } catch (const IOException& e) {
	LogFile::warning(e.what());
      }
    }
    m_nsm2ca->unlock();
  }
  count++;
}

void NSM2CACallback::vset(NSMCommunicator& com, const NSMVar& var) throw()
{
  NSMMessage msg(com.getMessage());
  std::string name = std::string(var.getNode()) + "." + var.getName();
  m_nsm2ca->notify(name, var);
}

void NSM2CACallback::vreply(NSMCommunicator& com, 
			    const std::string& vname, bool success) throw()
{
  const NSMMessage& msg(com.getMessage());
  std::string name = std::string(msg.getNodeName()) + "." + vname;
  m_nsm2ca->notify(name, msg);
}

void NSM2CACallback::error(const char* nodename, const char* data) throw()
{
  LogFile::error("Error from %s : %s", nodename, data);
  try {
    NSMVarPVTList& vars(m_nsm2ca->getVars());
    for (NSMVarPVTList::iterator it = vars.begin();
	 it != vars.end(); it++ ) {
      NSMVar& var((it->second).var);
      if (var.getNode() == nodename) {
	if (var.getName() == "rcstate") {
	  std::string name = var.getNode() + "." + var.getName();
	  m_nsm2ca->notify(name, NSMVar("rcstate", "NOTREADY"));
	  return;
	} else if (var.getName() == "state") {
	  std::string name = var.getNode() + "." + var.getName();
	  m_nsm2ca->notify(name, NSMVar("state", "ERROR"));
	  return;
	}
      }
    }
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
}

void NSM2CACallback::ok(const char* nodename, const char* data) throw()
{
  RCState rcstate(data);
  HVState hvstate(data);
  try {
    NSMVarPVTList& vars(m_nsm2ca->getVars());
    for (NSMVarPVTList::iterator it = vars.begin();
	 it != vars.end(); it++ ) {
      NSMVar& var((it->second).var);
      if (var.getNode() == nodename) {
	if (rcstate != RCState::UNKNOWN && var.getName() == "rcstate") {
	  std::string name = var.getNode() + "." + var.getName();
	  m_nsm2ca->notify(name, NSMVar("rcstate", data));
	  return;
	} else if (var.getName() == "state") {
	  std::string name = var.getNode() + "." + var.getName();
	  m_nsm2ca->notify(name, NSMVar("state", data));
	  return;
	}
      }
    }
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
}
