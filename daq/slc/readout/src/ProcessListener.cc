#include "daq/slc/readout/ProcessListener.h"

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/nsm/RCCallback.h"
#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

using namespace Belle2;

void ProcessListener::run()
{
  Fork forkid = _con->getFork();
  std::string process_name = _con->getName();
  if (forkid.wait() < 0) {
    Belle2::debug("[DEBU] Failed to wait for forked process %s", process_name.c_str());
    return;
  }
  _con->lock();
  NSMCommunicator* comm = _con->getCallback()->getCommunicator();
  NSMNode* node = _con->getCallback()->getNode();
  int state = _con->getMessanger().getState();
  switch (state) {
    case RunInfoBuffer::RUNNING:
      Belle2::debug("[ERROR] Forked process %s was crashed", process_name.c_str());
      comm->sendError(Belle2::form("Foked process %s was crashed", process_name.c_str()));
      break;
    case RunInfoBuffer::READY:
      Belle2::debug("[ERROR] Forked process %s was not started", process_name.c_str());
      comm->sendError(Belle2::form("Foked process %s was no started", process_name.c_str()));
      break;
    case RunInfoBuffer::NOTREADY:
    default:
      Belle2::debug("[INFO] Forked process %s was finished", process_name.c_str());
      comm->sendLog(SystemLog(node->getName(), SystemLog::INFO,
                              Belle2::form("Foked process %s was finished",
                                           process_name.c_str())));
      break;
  }
  _con->unlock();
}
