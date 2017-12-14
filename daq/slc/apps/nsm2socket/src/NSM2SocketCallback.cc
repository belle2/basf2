#include "daq/slc/apps/nsm2socket/NSM2SocketCallback.h"

#include "daq/slc/apps/nsm2socket/NSM2SocketBridge.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

NSM2SocketCallback::NSM2SocketCallback(const NSMNode& node)
throw() : NSMCallback(1)
{
  setNode(node);
  setTimeout(1);
}

bool NSM2SocketCallback::perform(NSMCommunicator& com) throw()
{
  NSMMessage msg(com.getMessage());
  NSMCommand cmd(msg.getRequestName());
  if (cmd == NSMCommand::VSET) {
    NSMCallback::perform(com);
  } else if (cmd == NSMCommand::LOGSET) {
    NSMCallback::perform(com);
  } else {
    m_bridge->send(msg);
  }
  return true;
}

void NSM2SocketCallback::logset(const DAQLogMessage& logmsg) throw()
{
  LogFile::put(logmsg.getPriority(), "%s :%s ",
               logmsg.getNodeName().c_str(), logmsg.getMessage().c_str());
  m_bridge->send(NSMMessage(NSMNode(logmsg.getNodeName()), logmsg));
}

void NSM2SocketCallback::requestLog() throw()
{
  try {
    NSMCommunicator::send(NSMMessage(getLogNode().getName(),
                                     NSMCommand::LOGGET));
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
}

void NSM2SocketCallback::vset(NSMCommunicator& com, const NSMVar& var) throw()
{
  NSMMessage& msg(com.getMessage());
  NSMNode node(msg.getNodeName());
  m_bridge->send(NSMMessage(node, NSMCommand::VSET), var);
}

NSMData& NSM2SocketCallback::getData(const std::string& name,
                                     const std::string& format,
                                     int revision) throw(NSMHandlerException)
{
  if (m_datas.find(name) == m_datas.end()) {
    NSMData data(name, format, revision);
    m_datas.insert(NSMDataMap::value_type(name, data));
  }
  return m_datas[name];
}

bool NSM2SocketCallback::send(const NSMMessage& msg) throw()
{
  m_mutex.lock();
  std::string name = StringUtil::toupper(msg.getNodeName());
  NSMNode node(name);
  if (m_nodes.find(name) == m_nodes.end()) {
    m_nodes.insert(NSMNodeMap::value_type(name, node));
  }
  try {
    NSMCommunicator::connected(node.getName());
  } catch (const NSMNotConnectedException&) {
    if (m_nodes[name].getState() != NSMState::UNKNOWN) {
      std::string emsg = StringUtil::form("Node %s is down.",
                                          node.getName().c_str());
      LogFile::error(emsg);
      DAQLogMessage log(getNode().getName(), LogFile::ERROR, emsg);
      m_bridge->send(NSMMessage(getNode().getName(), log));
    }
    m_nodes[name].setState(NSMState::UNKNOWN);
    m_mutex.unlock();
    return false;
  }
  try {
    if (NSMCommunicator::send(msg)) {
      m_nodes[name].setState(NSMState::ONLINE_S);
    } else {
      m_nodes[name].setState(NSMState::UNKNOWN);
    }
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
    DAQLogMessage log(getNode().getName(), LogFile::ERROR, e.what());
    m_bridge->send(NSMMessage(getNode(), log));
    m_mutex.unlock();
    return false;
  }
  m_mutex.unlock();
  return true;
}

void NSM2SocketCallback::timeout(NSMCommunicator& com) throw()
{
  static unsigned long long n = 0;
  for (NSMNodeMap::iterator it = m_nodes.begin();
       it != m_nodes.end(); it++) {
    NSMNode& node(it->second);
    try {
      NSMCommunicator::connected(node.getName());
    } catch (const NSMNotConnectedException&) {
      if (node.getState() != NSMState::UNKNOWN) {
        node.setState(NSMState::UNKNOWN);
        std::string emsg = StringUtil::form("Node %s got down.", node.getName().c_str());
        LogFile::error(emsg);
        DAQLogMessage log(getNode().getName(), LogFile::WARNING, emsg);
        m_bridge->send(NSMMessage(getNode(), log));
      }
    }
  }
  for (NSMDataMap::iterator it = m_datas.begin();
       it != m_datas.end(); it++) {
    NSMData& data(it->second);
    NSMMessage msg(getNode());
    try {
      bool opennew = false;
      if (!data.isAvailable()) {
        data.open(com);
        LogFile::info("open NSM data (%s) ", data.getName().c_str());
        opennew = true;
      }
      if (n % 5 == 0 || opennew) {
        msg.setNodeName(data.getName());
        msg.setRequestName(NSMCommand::DATASET);
        m_bridge->send(msg, data);
      }
    } catch (const NSMHandlerException& e) {
      //std::string log = StringUtil::form("Failed to open NSM data (%s:%s:%d) ",
      //                                   data.getName().c_str(),
      //                                   data.getFormat().c_str(),
      //                                   data.getRevision());
      //LogFile::warning(log);
      // m_bridge->send(DAQLogMessage(getNode().getName(), LogFile::WARNING, log));
    }
  }
  n++;
}

