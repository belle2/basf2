#include "LocalNSMCommunicator.hh"

#include "MessageBox.hh"

#include <nsm/NSMCommunicator.hh>

#include <util/Debugger.hh>

#include <iostream>

using namespace B2DAQ;

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
    B2DAQ::debug("[DEBUG] %s:%d: NSM error", __FILE__, __LINE__);
    throw (e);
  }
}

void LocalNSMCommunicator::sendMessage(const RunControlMessage& msg)
throw(IOException)
{
}
