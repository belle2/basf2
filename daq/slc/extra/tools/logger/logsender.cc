#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/base/Debugger.h>

#include <unistd.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./loggerd <name>");
    return 1;
  }
  const char* name = argv[1];
  NSMCommunicator* comm = new NSMCommunicator(new NSMNode(name));
  comm->init();
  while (true) {
    comm->sendLog(SystemLog(name, SystemLog::DEBUG, "degbug message"));
    sleep(5);
    comm->sendLog(SystemLog(name, SystemLog::INFO, "info message"));
    sleep(5);
    comm->sendLog(SystemLog(name, SystemLog::NOTICE, "notice message"));
    sleep(5);
    comm->sendLog(SystemLog(name, SystemLog::WARNING, "warning message"));
    sleep(5);
    comm->sendLog(SystemLog(name, SystemLog::ERROR,
                            "error message <a href=\"http://belle2.kek.jp\">http://belle2.kek.jp</a>"));
    sleep(5);
    comm->sendLog(SystemLog(name, SystemLog::FATAL, "fatal message"));
    sleep(5);
  }

  return 0;
}
