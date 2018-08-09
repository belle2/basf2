#include "daq/slc/apps/nsmbridged/NsmbridgeCallback.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include <daq/slc/system/LogFile.h>

#include <stdlib.h>

using namespace Belle2;

NsmbridgeCallback::NsmbridgeCallback(const std::string& name, int timeout)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)", name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);
  m_callback = NULL;
}

NsmbridgeCallback::~NsmbridgeCallback() throw()
{
}

void NsmbridgeCallback::init(NSMCommunicator&) throw()
{

}

void NsmbridgeCallback::timeout(NSMCommunicator&) throw()
{

}

void NsmbridgeCallback::vget(const std::string& nodename,
                             const std::string& vname) throw()
{
  LogFile::info("vget : " + vname);
  StringList s = StringUtil::split(vname, '@');
  if (s.size() >= 2) {
    NSMNode node(s[0]);
    std::string name = s[1];
    if (m_vlists.find(nodename) == m_vlists.end()) {
      m_vlists.insert(std::pair<std::string, NodeVlist>(nodename, NodeVlist()));
    }
    NodeVlist& list(m_vlists[nodename]);
    if (list.find(vname) == list.end()) {
      list.insert(std::pair<std::string, long long>(vname, 0));
    }
    if (m_vars.find(vname) != m_vars.end()) {
      m_vars.insert(std::pair<std::string, NSMVar>(vname, NSMVar()));
    }
    try {
      NSMCommunicator::send(NSMMessage(node, NSMCommand::VGET, name));
    } catch (const IOException& e) {
      LogFile::error(e.what());
    }
  }
}

void NsmbridgeCallback::vreply(NSMCommunicator& com,
                               const std::string& name, bool ret) throw()
{
  const NSMMessage& msg(com.getMessage());
  //LogFile::info("vreply : " + name);
  const std::string vname = StringUtil::form("%s@", msg.getNodeName()) + name;
  for (std::map<std::string, NodeVlist>::iterator it = m_vlists.begin();
       it != m_vlists.end(); it++) {
    NSMNode node(it->first);
    NodeVlist& list(it->second);
    if (list.find(vname) != list.end()) {
      //LogFile::info("reply : " + node.getName() + ">>" + vname);
      try {
        NSMCommunicator::send(NSMMessage(node, NSMCommand::VREPLY, ret, vname));
      } catch (const IOException& e) {
        LogFile::error(e.what());
      }
    }
  }
}

void NsmbridgeCallback::vset(NSMCommunicator& com, const NSMVar& var) throw()
{
  //LogFile::info("vset : " + var.getName());
  StringList s = StringUtil::split(var.getName(), '@');
  if (s.size() < 2) {
    //LogFile::debug("%s:%d", __FILE__, __LINE__);
    NSMMessage& msg(com.getMessage());
    std::string vname = StringUtil::form("%s@", msg.getNodeName()) + var.getName();
    NSMVar var_out;
    var_out = var;
    var_out.setNode(getNode().getName());
    var_out.setName(vname);
    for (std::map<std::string, NodeVlist>::iterator it = m_vlists.begin();
         it != m_vlists.end(); it++) {
      NSMNode node(it->first);
      NodeVlist& list(it->second);
      if (list.find(vname) != list.end()) {
        try {
          NSMCommunicator::send(NSMMessage(node, var_out));
        } catch (const IOException& e) {
          LogFile::error(e.what());
        }
      }
    }
  } else {
    NSMMessage& msg(com.getMessage());
    std::string nodename = msg.getNodeName();
    std::string vname = var.getName();
    NSMNode node(s[0]);
    std::string name = s[1];
    NSMVar var_out = var;
    var_out.setNode("");
    //var_out.setNode(node.getName());
    var_out.setName(name);
    if (m_vlists.find(nodename) == m_vlists.end()) {
      m_vlists.insert(std::pair<std::string, NodeVlist>(nodename, NodeVlist()));
    }
    NodeVlist& list(m_vlists[nodename]);
    if (list.find(vname) == list.end()) {
      list.insert(std::pair<std::string, long long>(vname, 0));
    }
    if (m_vars.find(vname) != m_vars.end()) {
      m_vars.insert(std::pair<std::string, NSMVar>(vname, var_out));
    }
    try {
      NSMCommunicator::send(NSMMessage(node, var_out));
      /*
      switch (var_out.getType()) {
      case NSMVar::INT:
      set(node, var_out.getName().c_str(), var_out.getInt());
      break;
      case NSMVar::FLOAT:
      set(node, var_out.getName().c_str(), var_out.getFloat());
      break;
      case NSMVar::TEXT:
      set(node, var_out.getName().c_str(), var_out.getText());
      break;
      default:
      break;
      }
      */
    } catch (const IOException& e) {
      LogFile::error(e.what());
    }
  }
}
