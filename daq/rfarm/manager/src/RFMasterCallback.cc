#include "daq/rfarm/manager/RFMasterCallback.h"

#include "daq/rfarm/manager/RFRunControlCallback.h"
#include "daq/rfarm/manager/RFCommand.h"
#include "daq/rfarm/manager/RfNodeInfo.h"
#include "daq/rfarm/manager/RfUnitInfo.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

using namespace Belle2;

bool RFMasterCallback::initialize(const DBObject& obj) throw()
{
  if (obj.hasObject("dqmserver")) {
    m_nodes.push_back(NSMNode(obj.getObject("dqmserver").getText("nodename")));
  }
  const std::string format = obj.getObject("system").getText("nsmdata");
  std::string nodename = obj.getObject("distributor").getText("nodename");
  m_nodes.push_back(NSMNode(nodename));
  m_datas.push_back(NSMData(nodename, format, 1));
  nodename = obj.getObject("collector").getText("nodename");
  m_nodes.push_back(NSMNode(nodename));
  m_datas.push_back(NSMData(nodename, format, 1));
  const DBObject& processor(obj.getObject("processor"));
  int maxnodes = processor.getInt("nnodes");
  int idbase = processor.getInt("idbase");
  std::string hostbase = processor.getText("nodebase");
  std::string badlist = processor.getText("badlist");
  char idname[3];
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    if (badlist.size() == 0 || StringUtil::find(badlist, idname)) {
      nodename = StringUtil::form("evp_%s%2.2d", hostbase.c_str(), idbase + i);
      m_nodes.push_back(NSMNode(nodename));
      m_datas.push_back(NSMData(nodename, format, 1));
    }
  }
  m_nodes.push_back(NSMNode(nodename));
  return configure(obj);
}

bool RFMasterCallback::configure(const DBObject&) throw()
{
  for (NSMNodeList::iterator it = m_nodes.begin();
       it != m_nodes.end(); it++) {
    NSMNode& node(*it);
    RCState s(node.getState());
    std::string vname = StringUtil::tolower(node.getName()) + ".rcstate";
    add(new NSMVHandlerText(vname, true, false, s.getLabel()));
  }
  return true;
}

void RFMasterCallback::setState(NSMNode& node, const RCState& state)
{
  node.setState(state);
  std::string vname = StringUtil::tolower(node.getName()) + ".rcstate";
  set(vname, state.getLabel());
}

void RFMasterCallback::timeout(NSMCommunicator& com) throw()
{
  if (!m_callback->getData().isAvailable()) return;
  RfUnitInfo* unitinfo = (RfUnitInfo*)m_callback->getData().get();
  unitinfo->nnodes = m_datas.size();
  unitinfo->updatetime = Time().get();
  unitinfo->rcstate = getNode().getState().getId();
  try {
    int i = 0;
    for (NSMDataList::iterator it = m_datas.begin();
         it != m_datas.end(); it++) {
      NSMData& data(*it);
      if (!data.isAvailable()) data.open(com);
      if (data.isAvailable()) {
        RfUnitInfo::RfNodeInfo* nodeinfo_o = &unitinfo->nodeinfo[i++];
        RfNodeInfo* nodeinfo_i = (RfNodeInfo*)data.get();
        memcpy(nodeinfo_o, nodeinfo_i, sizeof(RfNodeInfo));
      }
    }
  } catch (const NSMHandlerException& e) {}
}

void RFMasterCallback::ok(const char* nodename, const char* data) throw()
{
  for (NSMNodeList::iterator it = m_nodes.begin();
       it != m_nodes.end(); it++) {
    NSMNode& node(*it);
    if (node.getName() == nodename) {
      if (strcmp(data, "Configured") == 0) {
        setState(node, RCState::READY_S);
      } else if (strcmp(data, "Unconfigured") == 0) {
        setState(node, RCState::NOTREADY_S);
      } else if (strcmp(data, "Running") == 0) {
        setState(node, RCState::RUNNING_S);
      } else if (strcmp(data, "Idle") == 0) {
        setState(node, RCState::PAUSED_S);
      }
      LogFile::debug("OK << %s (%s)", nodename, data);
      return;
    }
  }
  LogFile::warning("OK from unknown node %s (%s)", nodename, data);
}

void RFMasterCallback::error(const char* nodename, const char* data) throw()
{
  for (NSMNodeList::iterator it = m_nodes.begin();
       it != m_nodes.end(); it++) {
    NSMNode& node(*it);
    if (node.getName() == nodename) {
      LogFile::error("ERROR << %s (%s)", nodename, data);
      setState(node, RCState::NOTREADY_S);
      return;
    }
  }
  LogFile::warning("Error from unknwon node %s : %s)", nodename, data);
}

void RFMasterCallback::load(const DBObject&) throw(RCHandlerException)
{
  for (NSMNodeList::iterator it = m_nodes.begin();
       it != m_nodes.end(); it++) {
    NSMNode& node(*it);
    if (NSMCommunicator::send(NSMMessage(node, RFCommand::UNCONFIGURE))) {
      setState(node, RCState::LOADING_TS);
      while (true) {
        NSMCommunicator& com(wait(node, NSMCommand::UNKNOWN));
        NSMMessage msg(com.getMessage());
        NSMCommand cmd(msg.getRequestName());
        if (cmd == NSMCommand::OK) {
          ok(msg.getNodeName(), msg.getData());
          break;
        } else {
          LogFile::debug("Unexpected reqeust : %s", msg.getRequestName());
          perform(com);
        }
      }
    } else {
      throw (RCHandlerException("Failed to configure %s", node.getName().c_str()));
    }
  }
}

void RFMasterCallback::abort() throw(RCHandlerException)
{
  for (NSMNodeList::reverse_iterator it = m_nodes.rbegin();
       it != m_nodes.rend(); it++) {
    NSMNode& node(*it);
    if (NSMCommunicator::send(NSMMessage(node, RFCommand::CONFIGURE))) {
      setState(node, RCState::ABORTING_RS);
      while (true) {
        NSMCommunicator& com(wait(node, NSMCommand::UNKNOWN));
        NSMMessage msg(com.getMessage());
        NSMCommand cmd(msg.getRequestName());
        if (cmd == NSMCommand::OK) {
          ok(msg.getNodeName(), msg.getData());
          break;
        } else {
          LogFile::debug("Unexpected reqeust : %s", msg.getRequestName());
          perform(com);
        }
      }
    } else {
      throw (RCHandlerException("Failed to configure %s", node.getName().c_str()));
    }
  }
}

void RFMasterCallback::start(int /*expno*/, int /*runno*/) throw(RCHandlerException)
{
}

void RFMasterCallback::stop() throw(RCHandlerException)
{
}

void RFMasterCallback::pause() throw(RCHandlerException)
{
}

void RFMasterCallback::resume() throw(RCHandlerException)
{
}

void RFMasterCallback::recover() throw(RCHandlerException)
{
}




