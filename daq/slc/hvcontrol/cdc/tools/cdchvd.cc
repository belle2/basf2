#include "daq/slc/hvcontrol/cdc/CDCHVControlCallback.h"
#include "daq/slc/hvcontrol/HVControlDaemon.h"

using namespace Belle2;

int main()
{
  HVControlDaemon(new CDCHVControlCallback(NSMNode("CDC_HV"))).run();
  return 0;
}
