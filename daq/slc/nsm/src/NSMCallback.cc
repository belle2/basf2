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
  }
  return false;
}

NSMMessage& NSMCallback::getMessage()
{
  return m_msg;
}

void NSMCallback::setMessage(NSMMessage& msg)
{
  m_msg = msg;
}

