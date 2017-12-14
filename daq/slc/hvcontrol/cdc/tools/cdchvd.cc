#include "daq/slc/hvcontrol/cdc/CdcHVControlCallback.h"
#include "daq/slc/hvcontrol/HVControlDaemon.h"

using namespace Belle2;

int main()
{
  HVControlDaemon(new CdcHVControlCallback(), "cdc").run();
  return 0;
}
