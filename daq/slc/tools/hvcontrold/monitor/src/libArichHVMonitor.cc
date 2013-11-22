#include "ArichHVMonitor.h"

extern "C" void* createArichHVMonitor()
{
  return new Belle2::ArichHVMonitor();
}

