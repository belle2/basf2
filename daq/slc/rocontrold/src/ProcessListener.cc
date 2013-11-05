#include "ProcessListener.h"

#include "nsm/RCCallback.h"
#include "nsm/NSMCommunicator.h"

#include "base/Debugger.h"
#include "base/StringUtil.h"

using namespace Belle2;

void ProcessListener::run()
{
  if (_forkfd.wait() < 0) {
    Belle2::debug("[DEBU] Failed to wait for forked process %s",
                  _process_name.c_str());
    return ;
  } else {
  }
  _mutex.lock();
  if (_is_running) {
    Belle2::debug("[ERROR] Forked process %s was down",
                  _process_name.c_str());
    try {
      _callback->getCommunicator()->replyError(Belle2::form("Process %s was done",
                                                            _process_name.c_str()));
    } catch (const IOException& e) {
      Belle2::debug("[DEBUG] %s:%d : exception=%s", __FILE__, __LINE__, e.what());
    }
  } else {
    Belle2::debug("[INFO] Forked process %s was finished",
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
