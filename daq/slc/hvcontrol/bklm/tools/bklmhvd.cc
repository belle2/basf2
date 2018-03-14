#include "daq/slc/hvcontrol/bklm/BklmHVControlCallback.h"
#include "daq/slc/hvcontrol/HVControlDaemon.h"

using namespace Belle2;

int main()
{
  HVControlDaemon(new BklmHVControlCallback(), "bklm").run();

  return 0;
}
