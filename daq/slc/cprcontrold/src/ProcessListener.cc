#include "ProcessListener.hh"

#include "COPPERCallback.hh"

#include <util/Debugger.hh>

#include <nsm/NSMCommunicator.hh>

#include <runcontrol/RCState.hh>

using namespace B2DAQ;

void ProcessListener::run()
{
  if (_forkfd.wait() < 0) {
    B2DAQ::debug("Failed to wait for forked process");
    return ;
  } else {
  }
  _mutex.lock();
  if (_is_running) {
    B2DAQ::debug("Forked process was down");
    _callback->getNode()->setState(RCState::ERROR_ES);
    try {
      _callback->getCommunicator()->sendError(_callback->getNode(),
                                              "Readout process was done");
    } catch (const IOException& e) {
      B2DAQ::debug("%s:%d : exception=%s", __FILE__, __LINE__, e.what());
    }
  } else {
    B2DAQ::debug("Forked process was finished");
  }
  _is_running = false;
  _mutex.unlock();
}

void ProcessListener::setRunning(bool is_running)
{
  _mutex.lock();
  _is_running = is_running;
  _mutex.unlock();
}
