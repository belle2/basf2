#include "daq/slc/hvcontrol/arich/ArichHVControlCallback.h"
#include "daq/slc/hvcontrol/HVControlDaemon.h"

using namespace Belle2;

int main()
{
  HVControlDaemon(new ArichHVControlCallback(),
                  "arich").run();
  return 0;
}
