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
  if (_con->getState() == State::LOADING_TS) {
    Belle2::debug("[ERROR] Forked process %s was not started", process_name.c_str());
    _con->getLog().send(SystemLog::ERROR, process_name + " was not started");
  } else if (_con->getState() == State::RUNNING_S) {
    Belle2::debug("[ERROR] Forked process %s was crashed", process_name.c_str());
    _con->getLog().send(SystemLog::ERROR, process_name + " was crashed");
  } else {
    Belle2::debug("[INFO] Forked process %s was finished", process_name.c_str());
    _con->getLog().send(SystemLog::NOTICE, process_name + " was finished");
  }
  _con->unlock();
}
