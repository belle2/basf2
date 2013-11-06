#include "nsm/LogCallback.h"
#include "nsm/NSMCommunicator.h"

#include "base/Debugger.h"
#include "base/ConfigFile.h"

using namespace Belle2;

bool LogCallback::perform(NSMMessage& msg) throw(NSMHandlerException)
{
  Command cmd(msg.getRequestName());
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
{
  _node = node;
  add(Command::LOG);
}
