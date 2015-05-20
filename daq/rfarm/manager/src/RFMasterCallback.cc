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

RFMasterCallback::RFMasterCallback() throw() : m_callback(NULL)
{
  reg(RFCommand::CONFIGURE);
  reg(RFCommand::UNCONFIGURE);
  reg(RFCommand::START);
  reg(RFCommand::STOP);
  reg(RFCommand::RESUME);
  reg(RFCommand::PAUSE);
  reg(RFCommand::RESTART);
  reg(RFCommand::STATUS);
}

void RFMasterCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  configure(obj);
}

void RFMasterCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  m_nodes = NSMNodeList();
  m_dataname = StringList();
  if (obj.hasObject("dqmserver")) {
    m_nodes.push_back(NSMNode(obj("dqmserver").getText("nodename")));
  }
  const std::string format = obj("system").getText("nsmdata");
  std::string nodename = obj("distributor").getText("nodename");
  m_nodes.push_back(NSMNode(nodename));
  addData(nodename, format);
  nodename = obj("collector").getText("nodename");
  m_nodes.push_back(NSMNode(nodename));
  addData(nodename, format);
  const DBObject& processor(obj("processor"));
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
      addData(nodename, format);
    }
  }
  add(new NSMVHandlerInt("nnodes", true, false, m_nodes.size()));
  //m_nodes.push_back(NSMNode(nodename));
  int i = 0;
  std::string rcconf = obj.getName();
  if (StringUtil::find(rcconf, "@")) {
    rcconf = StringUtil::split(rcconf, '@')[1];
  }
  for (NSMNodeList::iterator it = m_nodes.begin();
       it != m_nodes.end(); it++) {
    NSMNode& node(*it);
    RCState s(node.getState());
    std::string vname = StringUtil::form("node[%d].name", i++);
    add(new NSMVHandlerText(vname, true, false,
                            StringUtil::tolower(node.getName())));
    vname = StringUtil::tolower(node.getName());
    add(new NSMVHandlerText(vname + ".rcstate", true, false, s.getLabel()));
    add(new NSMVHandlerText(vname + ".rcconfig", true, false, rcconf));
  }
}

void RFMasterCallback::setState(NSMNode& node, const RCState& state)
{
  node.setState(state);
  std::string vname = StringUtil::tolower(node.getName()) + ".rcstate";
  set(vname, state.getLabel());
}

void RFMasterCallback::monitor() throw(RCHandlerException)
{
  if (!m_callback->getData().isAvailable()) return;
  RfUnitInfo* unitinfo = (RfUnitInfo*)m_callback->getData().get();
  unitinfo->nnodes = m_dataname.size();
  unitinfo->updatetime = Time().get();
  unitinfo->rcstate = getNode().getState().getId();
  int i = 0;
  for (StringList::iterator it = m_dataname.begin();
       it != m_dataname.end(); it++) {
    NSMData& data(getData(*it));
    try {
      if (data.isAvailable()) {
        RfUnitInfo::RfNodeInfo* nodeinfo_o = &unitinfo->nodeinfo[i];
        RfNodeInfo* nodeinfo_i = (RfNodeInfo*)data.get();
        memcpy(nodeinfo_o, nodeinfo_i, sizeof(RfNodeInfo));
      }
    } catch (const NSMHandlerException& e) {}
    i++;
  }
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
  for (NSMNodeList::iterator it = m_nodes.begin(); it != m_nodes.end(); it++) {
    NSMNode& node(*it);
    if (NSMCommunicator::send(NSMMessage(node, RFCommand::CONFIGURE))) {
      setState(node, RCState::LOADING_TS);
      while (node.getName().find("EVP") == std::string::npos &&
             node.getState() != RCState::READY_S) {
        try {
          perform(NSMCommunicator::select(60));
        } catch (const TimeoutException& e) {
          throw (RCHandlerException("Failed to configure due to timeout from %s",
                                    node.getName().c_str()));
        }
      }
    } else {
      throw (RCHandlerException("Failed to configure %s", node.getName().c_str()));
    }
  }
  while (true) {
    bool configured = true;
    for (NSMNodeList::iterator it = m_nodes.begin(); it != m_nodes.end(); it++) {
      if (it->getState() != RCState::READY_S) configured = false;
    }
    if (configured) return;
    try {
      perform(NSMCommunicator::select(30));
    } catch (const TimeoutException& e) {}
  }
}

void RFMasterCallback::abort() throw(RCHandlerException)
{
  for (NSMNodeList::reverse_iterator it = m_nodes.rbegin();
       it != m_nodes.rend(); it++) {
    NSMNode& node(*it);
    if (NSMCommunicator::send(NSMMessage(node, RFCommand::UNCONFIGURE))) {
      setState(node, RCState::ABORTING_RS);
      if (node.getName().find("EVP") == std::string::npos) {
        while (true) {
          bool unconfigured = (node.getState() == RCState::NOTREADY_S);
          for (NSMNodeList::reverse_iterator it2 = m_nodes.rbegin();
               it2 != it; it2++) {
            if (it2->getState() != RCState::NOTREADY_S)
              unconfigured = false;
          }
          if (unconfigured) break;
          try {
            perform(NSMCommunicator::select(30));
          } catch (const TimeoutException& e) {}
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

void RFMasterCallback::resume(int /*subno*/) throw(RCHandlerException)
{
}

void RFMasterCallback::recover(const DBObject& obj) throw(RCHandlerException)
{
  abort();
  load(obj);
}

void RFMasterCallback::addData(const std::string& dataname, const std::string& format)
{
  openData(dataname, format);
  int i = (int)m_dataname.size();
  std::string vname = StringUtil::form("data[%d].name", i);
  add(new NSMVHandlerText(vname, true, false, StringUtil::tolower(dataname)));
  vname = StringUtil::form("data[%d].format", i);
  add(new NSMVHandlerText(vname, true, false, format));
  m_dataname.push_back(dataname);
}

bool RFMasterCallback::perform(NSMCommunicator& com) throw()
{
  const NSMMessage msg = com.getMessage();
  if (RCCallback::perform(com)) {
    return true;
  }
  const RFCommand cmd = msg.getRequestName();
  try {
    if (cmd == RFCommand::CONFIGURE) {
      load(getDBObject());
      return true;
    } else if (cmd == RFCommand::UNCONFIGURE) {
      abort();
      return true;
    } else if (cmd == RFCommand::START) {
      return true;
    } else if (cmd == RFCommand::STOP) {
      return true;
    } else if (cmd == RFCommand::RESTART) {
      return true;
    } else if (cmd == RFCommand::PAUSE) {
      return true;
    } else if (cmd == RFCommand::RESUME) {
      return true;
    } else if (cmd == RFCommand::STATUS) {
      return true;
    }
  } catch (const RCHandlerException& e) {
    std::string emsg = StringUtil::form("%s. aborting", e.what());
    LogFile::error(emsg);
    reply(NSMMessage(NSMCommand::ERROR, emsg));
    try {
      RCCallback::setState(RCState::ABORTING_RS);
      abort();
      RCCallback::setState(RCState::NOTREADY_S);
    } catch (const RCHandlerException& e) {
      std::string emsg = StringUtil::form("Failed to abort : %s", e.what());
      LogFile::fatal(emsg);
    }
  } catch (const std::exception& e) {
    LogFile::fatal("Unknown exception: %s. terminating process", e.what());
  }
  return false;
}
