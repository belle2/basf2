#include "daq/slc/nsm/LogCallback.h"
#include "daq/slc/nsm/NSMCommunicator.h"

using namespace Belle2;

bool LogCallback::perform(NSMMessage& msg)
throw(NSMHandlerException)
{
  const Command cmd = msg.getRequestName();
  setReply("");
  if (cmd == Command::OK) {
    return ok();
  } else if (cmd == Command::ERROR) {
    return error();
  } else if (cmd == Command::LOG) {
    return log();
  }
  return true;
}

LogCallback::LogCallback(NSMNode* node) throw()
  : NSMCallback(node)
{
  add(Command::LOG);
}
