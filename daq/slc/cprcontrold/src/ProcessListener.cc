#include "ProcessListener.hh"

#include <util/Debugger.hh>

using namespace B2DAQ;

void ProcessListener::run()
{
  if (_forkfd.wait() < 0) {
    B2DAQ::debug("Failed to wait for forked process");
  } else {
    B2DAQ::debug("Forked process was gone");
  }
}
