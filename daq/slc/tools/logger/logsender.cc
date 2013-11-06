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
  NSMCommunicator* comm = new NSMCommunicator(new NSMNode(argv[1]));
  comm->init();
  comm->sendLog(LogMessage(LogMessage::DEBUG, "degbug message"));
  comm->sendLog(LogMessage(LogMessage::INFO, "info message"));

  return 0;
}
