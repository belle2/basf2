#include "daq/slc/hvcontrol/arichlv/ArichlvControlCallback.h"
#include "daq/slc/hvcontrol/HVControlDaemon.h"

using namespace Belle2;

int main()
{
  HVControlDaemon(new ArichlvControlCallback(), "arichlv").run();
  return 0;
}
