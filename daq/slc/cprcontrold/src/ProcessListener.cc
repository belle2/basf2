#include "ProcessListener.h"

#include "COPPERCallback.h"

#include <base/Debugger.h>

#include <nsm/NSMCommunicator.h>

using namespace Belle2;

void ProcessListener::run()
{
  if (_forkfd.wait() < 0) {
    Belle2::debug("[DEBU] Failed to wait for forked process");
    return ;
  } else {
  }
  _mutex.lock();
  if (_is_running) {
    Belle2::debug("[ERROR] Forked process was down");
    try {
      _callback->getCommunicator()->replyError("Readout process was done");
    } catch (const IOException& e) {
      Belle2::debug("[DEBUG] %s:%d : exception=%s", __FILE__, __LINE__, e.what());
    }
  } else {
    Belle2::debug("[INFO] Forked process was finished");
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
