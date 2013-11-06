#include "nsm/LogCallback.h"
#include "nsm/NSMCommunicator.h"

#include "base/Debugger.h"
#include "base/ConfigFile.h"

using namespace Belle2;

bool LogCallback::perform(NSMMessage& msg) throw(NSMHandlerException)
{
  Command cmd(msg.getRequestName());
  if (cmd == Command::OK) {
    return ok();
  } else if (cmd == Command::ERROR) {
    return error();
  }
  setReply("");
  NSMCommunicator* com = getCommunicator();
  bool result = true;
  if (cmd == Command::LOG) {
    if ((result = log())) {
      //com->replyOK(_node, _reply);
    } else {
      //com->replyError(_reply);
    }
  }
  return result;
}

LogCallback::LogCallback(NSMNode* node) throw()
{
  _node = node;
  add(Command::LOG);
}
