#include "ProcessListener.hh"

#include <runcontrol/RCCallback.hh>

#include <nsm/NSMCommunicator.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

using namespace B2DAQ;

void ProcessListener::run()
{
  if (_forkfd.wait() < 0) {
    B2DAQ::debug("[DEBU] Failed to wait for forked process %s",
                 _process_name.c_str());
    return ;
  } else {
  }
  _mutex.lock();
  if (_is_running) {
    B2DAQ::debug("[ERROR] Forked process %s was down",
                 _process_name.c_str());
    try {
      _callback->getCommunicator()->replyError(B2DAQ::form("Process %s was done",
                                                           _process_name.c_str()));
    } catch (const IOException& e) {
      B2DAQ::debug("[DEBUG] %s:%d : exception=%s", __FILE__, __LINE__, e.what());
    }
  } else {
    B2DAQ::debug("[INFO] Forked process %s was finished",
                 _process_name.c_str());
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
