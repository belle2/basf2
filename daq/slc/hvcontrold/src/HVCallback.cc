#include "HVCallback.h"

#include "HVCommand.h"
#include "HVState.h"

#include "nsm/NSMCommunicator.h"

#include "system/TCPSocketReader.h"

#include "base/Debugger.h"
#include "base/ConfigFile.h"

using namespace Belle2;

void HVCallback::download() throw()
{
  TCPSocket socket;
  try {
    ConfigFile config("slc_config");
    socket.connect(config.get("HV_GLOBAL_HOST"),
                   config.getInt("HV_GLOBAL_PORT"));
    TCPSocketReader reader(socket);
    m_crate->readObject(reader);
  } catch (const IOException& e) {
    Belle2::debug("Socket Erorr: %s", e.what());
  }
  socket.close();
}
bool HVCallback::perform(NSMMessage& msg) throw(NSMHandlerException)
{
  HVCommand cmd(msg.getRequestName());
  setReply("");
  if (cmd == Command::OK) {
    return ok();
  } else if (cmd == Command::ERROR) {
    return error();
  }
  /*
  if (cmd.isAvailable(_node->getState()) == 0) {
    return false;
  }
  */
  State state_org = _node->getState();
  bool result = false;
  NSMCommunicator* com = getCommunicator();
  if (cmd == HVCommand::LOAD) {
    download();
    result = load();
  } else if (cmd == HVCommand::SWITCHON) {
    result = switchOn();
  } else if (cmd == HVCommand::SWITCHOFF) {
    result = switchOff();
  } else if (cmd == Command::STATECHECK) {
    com->replyOK(_node, "");
    return true;
  }
  if (result) {
    com->replyOK(_node, _reply);
    return true;
  } else {
    _node->setState(State::ERROR_ES);
    com->replyError(_reply);
  }
  return false;
}

HVCallback::HVCallback(NSMNode* node) throw()
  : NSMCallback(node)
{
  m_crate = new HVCrateInfo();
  add(HVCommand::LOAD);
  add(HVCommand::SWITCHON);
  add(HVCommand::SWITCHOFF);
  add(Command::STATECHECK);
}
