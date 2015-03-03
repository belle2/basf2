#include "daq/slc/hvcontrol/template/TemplateHVControlCallback.h"
#include "daq/slc/hvcontrol/HVControlDaemon.h"

using namespace Belle2;

int main()
{
  HVControlDaemon(new TemplateHVControlCallback(), "template").run();
  return 0;
}
