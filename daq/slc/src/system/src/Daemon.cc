#include "Daemon.hh"

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

using namespace B2DAQ;

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


