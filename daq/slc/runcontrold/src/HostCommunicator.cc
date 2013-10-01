#include "HostCommunicator.hh"
#include "MessageBox.hh"

#include <util/Debugger.hh>

#include <unistd.h>

using namespace B2DAQ;

void HostCommunicator::run()
{
  while (true) {
    while (true) {
      try {
        init();
        break;
      } catch (const IOException& e) {
        B2DAQ::debug("[DEBUG] %s:%d:Failed to init %s", __FILE__, __LINE__, e.what());
        sleep(5);
      }
    }
    try {
      while (true) {
        RunControlMessage msg = waitMessage();
        MessageBox::get().push(msg);
      }
    } catch (const IOException& e) {
      B2DAQ::debug("[DEBUG] %s:%d:Failed to recieve Message", __FILE__, __LINE__);
    }
    reset();
  }
}
