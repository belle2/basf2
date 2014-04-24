#include "daq/slc/hvcontrol/arich/ArichHVControlCallback.h"
#include "daq/slc/hvcontrol/HVControlDaemon.h"

using namespace Belle2;

int main()
{
  HVControlDaemon(new ArichHVControlCallback(NSMNode("ARICH_HV"))).run();
  return 0;
}
