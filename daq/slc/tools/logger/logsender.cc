#include "LoggerCallback.h"

#include <nsm/NSMCommunicator.h>

#include <base/Debugger.h>

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
    comm->sendLog(LogMessage(name, LogMessage::DEBUG, "degbug message"));
    sleep(5);
    comm->sendLog(LogMessage(name, LogMessage::INFO, "info message"));
    sleep(5);
    comm->sendLog(LogMessage(name, LogMessage::NOTICE, "notice message"));
    sleep(5);
    comm->sendLog(LogMessage(name, LogMessage::WARNING, "warning message"));
    sleep(5);
    comm->sendLog(LogMessage(name, LogMessage::ERROR,
                             "error message <a href=\"http://belle2.kek.jp\">http://belle2.kek.jp</a>"));
    sleep(5);
    comm->sendLog(LogMessage(name, LogMessage::FATAL, "fatal message"));
    sleep(5);
  }

  return 0;
}
