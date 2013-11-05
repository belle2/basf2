#include "HostCommunicator.h"
#include "MessageBox.h"

#include <base/Debugger.h>

#include <unistd.h>

using namespace Belle2;

void HostCommunicator::run()
{
  while (true) {
    while (true) {
      try {
        init();
        break;
      } catch (const IOException& e) {
        Belle2::debug("[DEBUG] %s:%d:Failed to init %s", __FILE__, __LINE__, e.what());
        sleep(5);
      }
    }
    try {
      while (true) {
        RunControlMessage msg = waitMessage();
        MessageBox::get().push(msg);
      }
    } catch (const IOException& e) {
    }
    reset();
  }
}
