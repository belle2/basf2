#include "daq/slc/system/Daemon.h"

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

using namespace Belle2;

Daemon::Daemon()
{

}

Daemon::~Daemon() throw()
{

}

int Daemon::start() throw()
{
  if (daemon(0, 0) < 0) {
    exit(-1);
  }
  run();
  return ::getpid();
}


