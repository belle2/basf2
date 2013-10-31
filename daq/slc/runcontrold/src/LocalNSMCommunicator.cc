#include "LocalNSMCommunicator.h"

#include "MessageBox.h"

#include <nsm/NSMCommunicator.h>

#include <base/Debugger.h>

#include <iostream>

using namespace Belle2;

bool LocalNSMCommunicator::init() throw(IOException)
{
  return true;
}

bool LocalNSMCommunicator::reset() throw()
{
  return true;
}

RunControlMessage LocalNSMCommunicator::waitMessage() throw(IOException)
{
  try {
    RunControlMessage msg(RunControlMessage::LOCALNSM);
    if (_nsm_comm->wait(5000)) {
      msg.setMessage(_nsm_comm->getMessage());
      msg.setCommand(msg.getMessage().getRequestName());
    } else {
      msg.setCommand(Command::STATECHECK);
    }
    return msg;
  } catch (const IOException& e) {
    Belle2::debug("[DEBUG] %s:%d: NSM error", __FILE__, __LINE__);
    throw (e);
  }
}

void LocalNSMCommunicator::sendMessage(const RunControlMessage& msg)
throw(IOException)
{
}
