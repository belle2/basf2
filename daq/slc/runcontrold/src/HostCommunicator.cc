#include "HostCommunicator.hh"
#include "MessageBox.hh"

#include <util/Debugger.hh>

#include <unistd.h>

using namespace B2DAQ;

void HostCommunicator::run()
{
  B2DAQ::debug("waiting..");
  while (true) {
    while (true) {
      try {
        B2DAQ::debug("%s:%d tring to init", __FILE__, __LINE__);
        init();
        break;
      } catch (const IOException& e) {
        B2DAQ::debug("%s:%d:Failed to init", __FILE__, __LINE__);
        sleep(5);
      }
    }
    try {
      while (true) {
        RunControlMessage msg = waitMessage();
        MessageBox::get().push(msg);
      }
    } catch (const IOException& e) {
      B2DAQ::debug("%s:%d:Failed to recieve Message", __FILE__, __LINE__);
    }
    B2DAQ::debug("%s:%d closed communication", __FILE__, __LINE__);
    reset();
  }
}
