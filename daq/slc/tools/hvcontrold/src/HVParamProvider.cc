#include "HVParamProvider.h"

#include "HVControlMaster.h"

#include <unistd.h>

using namespace Belle2;

void HVParamProvider::run()
{
  _master->accept(_socket);
  sleep(5);
}

