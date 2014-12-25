#include "daq/slc/apps/templated/MasterCallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

MasterCallback::MasterCallback(const NSMNode& node)
  : RCCallback(node, 5)
{
}

MasterCallback::~MasterCallback() throw()
{
}

void MasterCallback::init() throw()
{
}

void MasterCallback::term() throw()
{
}

void MasterCallback::timeout() throw()
{
}

bool MasterCallback::ok() throw()
{
  NSMMessage& msg(getMessage());
  m_count++;
  LogFile::debug("OK from %s (state = %s) count = %d",
                 msg.getNodeName(), msg.getData(), m_count);
  return true;
}

bool MasterCallback::load() throw()
{
  NSMCommunicator& com(*getCommunicator());
  for (int i = 0; i < 20; i++) {
    std::string nodename = StringUtil::form("NODE%d", i);
    NSMMessage msg(NSMMessage(NSMNode(nodename), RCCommand::LOAD));
    com.sendRequest(msg);
  }
  m_count = 0;
  return true;
}

bool MasterCallback::trigft() throw()
{
  return true;
}

bool MasterCallback::start() throw()
{
  NSMCommunicator& com(*getCommunicator());
  for (int i = 0; i < 20; i++) {
    std::string nodename = StringUtil::form("NODE%d", i);
    NSMMessage msg(NSMMessage(NSMNode(nodename), RCCommand::START));
    com.sendRequest(msg);
  }
  m_count = 0;
  return true;
}

bool MasterCallback::stop() throw()
{
  NSMCommunicator& com(*getCommunicator());
  for (int i = 0; i < 20; i++) {
    std::string nodename = StringUtil::form("NODE%d", i);
    NSMMessage msg(NSMMessage(NSMNode(nodename), RCCommand::STOP));
    com.sendRequest(msg);
  }
  m_count = 0;
  return true;
}

bool MasterCallback::resume() throw()
{
  return true;
}

bool MasterCallback::pause() throw()
{
  return true;
}

bool MasterCallback::recover() throw()
{
  getNode().setState(RCState::READY_S);
  return false;
}

bool MasterCallback::abort() throw()
{
  NSMCommunicator& com(*getCommunicator());
  for (int i = 0; i < 20; i++) {
    std::string nodename = StringUtil::form("NODE%d", i);
    NSMMessage msg(NSMMessage(NSMNode(nodename), RCCommand::ABORT));
    com.sendRequest(msg);
  }
  getNode().setState(RCState::NOTREADY_S);
  m_count = 0;
  return true;
}

