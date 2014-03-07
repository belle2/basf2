#include "daq/slc/apps/hvcontrold/arich/ArichHVControlCallback.h"
#include "daq/slc/apps/hvcontrold/HVControlDaemon.h"

using namespace Belle2;

int main()
{
  HVControlDaemon(new ArichHVControlCallback(new NSMNode("ARICH_HV"))).run();
  return 0;
}
