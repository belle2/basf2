#include "daq/slc/nsm/NSMCallback.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include <signal.h>
#include <string.h>
#include <cstdlib>

using namespace Belle2;

NSMCallback::NSMCallback(const NSMNode& node, int timeout) throw()
  : m_node(node), m_comm(NULL)
{
  add(NSMCommand::OK);
  add(NSMCommand::ERROR);
  add(NSMCommand::FATAL);
  add(NSMCommand::LOG);
  add(NSMCommand::STATE);
  add(NSMCommand::EXCLUDE);
  add(NSMCommand::INCLUDE);
  add(NSMCommand::VGET);
  add(NSMCommand::VSET);
  m_timeout = timeout;
}

bool NSMCallback::isReady() const throw()
{
  return m_comm != NULL && m_comm->isOnline();
}

bool NSMCallback::perform(const NSMMessage& msg)
throw()
{
  const NSMCommand cmd = msg.getRequestName();
  if (cmd == NSMCommand::OK) {
    return ok();
  } else if (cmd == NSMCommand::ERROR) {
    return error();
  } else if (cmd == NSMCommand::FATAL) {
    return fatal();
  } else if (cmd == NSMCommand::LOG) {
    return log();
  } else if (cmd == NSMCommand::STATE) {
    return state();
  } else if (cmd == NSMCommand::EXCLUDE) {
    return exclude();
  } else if (cmd == NSMCommand::INCLUDE) {
    return include();
  } else if (cmd == NSMCommand::VGET) {
    std::string vname = msg.getData();
    NSMVar var(vget(vname));
    if (var.getType() != NSMVar::NONE) {
      var.setName(vname);
      m_comm->sendRequest(NSMMessage(NSMNode(msg.getNodeName()), var));
    }
    return true;
  } else if (cmd == NSMCommand::VSET) {
    std::string vname = msg.getData();
    const char* value = (msg.getData() + msg.getParam(3) + 1);
    vset(NSMVar(vname, (NSMVar::Type)msg.getParam(0),
                msg.getParam(1), value));
    return true;
  }
  return false;
}

NSMMessage& NSMCallback::getMessage()
{
  return m_msg;
}

const NSMMessage& NSMCallback::getMessage() const
{
  return m_msg;
}

void NSMCallback::setMessage(const NSMMessage& msg)
{
  m_msg = msg;
}

