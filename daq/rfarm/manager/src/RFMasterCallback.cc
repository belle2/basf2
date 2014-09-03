#include "daq/rfarm/manager/RFMasterCallback.h"
#include "daq/rfarm/manager/RFRunControlCallback.h"

#include "daq/rfarm/manager/RFMaster.h"
#include "daq/rfarm/manager/RFCommand.h"
#include "daq/rfarm/manager/RFNSM.h"

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RfNodeInfo.h"
#include "daq/rfarm/manager/RfUnitInfo.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

using namespace Belle2;

RFMasterCallback::RFMasterCallback(const NSMNode& node,
                                   const NSMData& data,
                                   RFMaster* master,
                                   const char* confile)
  : NSMCallback(node), m_data(data), m_master(master), m_conf(confile)
{
  m_st_conf = 0;
  m_st_unconf = 0;
  add(RFCommand::RF_CONFIGURE);
  add(RFCommand::RF_UNCONFIGURE);
  add(RFCommand::RF_START);
  add(RFCommand::RF_STOP);
  add(RFCommand::RF_RESTART);
  add(RFCommand::RF_RESUME);
  add(RFCommand::RF_PAUSE);
  add(RFCommand::RF_STATUS);
}

RFMasterCallback::~RFMasterCallback() throw()
{

}

void RFMasterCallback::init() throw()
{
  m_master->SetNodeInfo((RfNodeInfo*)m_data.allocate(getCommunicator(), true));
  char* node = m_conf.getconf("dqmserver", "nodename");
  m_nodes.insert(NSMNodeList::value_type(node, NSMNode(node)));
  node = m_conf.getconf("distributor", "nodename");
  m_nodes.insert(NSMNodeList::value_type(node, NSMNode(node)));
  char* format = m_conf.getconf("system", "nsmdata");
  m_data_v.push_back(NSMData(node, format, 1));
  node = m_conf.getconf("collector", "nodename");
  m_nodes.insert(NSMNodeList::value_type(node, NSMNode(node)));
  m_data_v.push_back(NSMData(node, format, 1));
  int maxnodes = m_conf.getconfi("processor", "nnodes");
  int idbase = m_conf.getconfi("processor", "idbase");
  char* hostbase = m_conf.getconf("processor", "nodebase");
  char* badlist = m_conf.getconf("processor", "badlist");
  char hostnode[512], idname[3];
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    if (badlist == NULL ||
        strstr(badlist, idname) == 0) {
      sprintf(hostnode, "evp_%s%2.2d", hostbase, idbase + i);
      m_data_v.push_back(NSMData(hostnode, format, 1));
      m_nodes.insert(NSMNodeList::value_type(hostnode, NSMNode(hostnode)));
    }
  }
  node = m_conf.getconf("roisender", "nodename");
  m_nodes.insert(NSMNodeList::value_type(node, NSMNode(node)));
  for (NSMNodeIterator it = m_nodes.begin();
       it != m_nodes.end(); it++) {
    it->second.setState(RCState::NOTREADY_S);
  }
}

bool RFMasterCallback::perform(const NSMMessage& msg) throw()
{
  RFCommand cmd(msg.getRequestName());
  setReply("");
  if (cmd == NSMCommand::OK) {
    return ok();
  } else if (cmd == NSMCommand::ERROR) {
    return error();
  }
  bool result = false;
  LogFile::debug("RFMasterCallback : %s >> %s",
                 msg.getNodeName(),
                 cmd.getLabel());
  if (cmd == RFCommand::RF_CONFIGURE) {
    getNode().setState(RCState::LOADING_TS);
    return configure();
  } else if (cmd == RFCommand::RF_UNCONFIGURE) {
    getNode().setState(RCState::ABORTING_RS);
    return unconfigure();
  } else if (cmd == RFCommand::RF_START) {
    result = start();
  } else if (cmd == RFCommand::RF_STOP) {
    result = stop();
  } else if (cmd == RFCommand::RF_RESTART) {
    result = restart();
  } else if (cmd == RFCommand::RF_PAUSE) {
    result = pause();
  } else if (cmd == RFCommand::RF_RESUME) {
    result = resume();
  } else if (cmd == RFCommand::RF_STATUS) {
    result = status();
  }
  reply(result);
  return true;
}

void RFMasterCallback::reply(bool result)
{
  NSMCommunicator& com(*getCommunicator());
  NSMCommunicator& com_g(*m_callback->getCommunicator());
  if (result) {
    getNode().setError(0);
    com.replyOK(getNode());
    com_g.replyOK(getNode());
  } else {
    com.replyError(getNode().getError(), getReply());
    com_g.replyError(getNode().getError(), getReply());
  }
}

void RFMasterCallback::timeout() throw()
{
  if (!m_callback->getData().isAvailable()) return;
  RfUnitInfo* unitinfo = (RfUnitInfo*)m_callback->getData().get();
  unitinfo->nnodes = m_data_v.size();
  unitinfo->updatetime = Time().get();
  try {
    for (size_t i = 0; i < m_data_v.size(); i++) {
      if (!m_data_v[i].isAvailable()) {
        m_data_v[i].open(getCommunicator(), true);
      }
      if (m_data_v[i].isAvailable()) {
        RfUnitInfo::RfNodeInfo* nodeinfo_o = &unitinfo->nodeinfo[i];
        RfNodeInfo* nodeinfo_i = (RfNodeInfo*)m_data_v[i].get();
        memcpy(nodeinfo_o, nodeinfo_i, sizeof(RfNodeInfo));
      }
    }
  } catch (const NSMHandlerException& e) {

  }
}

bool RFMasterCallback::ok() throw()
{
  const NSMMessage& msg(getMessage());
  LogFile::debug("%s << %s (%s)",
                 msg.getRequestName(),
                 msg.getNodeName(), msg.getData());
  std::string nodename = msg.getNodeName();
  if (m_nodes.find(nodename) == m_nodes.end() ||
      msg.getLength() == 0) {
    return true;
  }
  NSMNode& node(m_nodes[nodename]);
  std::string data = msg.getData();
  if (data == "Configured") {
    node.setState(RCState::READY_S);
    return configure();
  } else if (data == "Unconfigured") {
    node.setState(RCState::NOTREADY_S);
    return unconfigure();
  } else if (data == "Running") {
    node.setState(RCState::RUNNING_S);
  } else if (data == "Idle") {
    node.setState(RCState::PAUSED_S);
  }
  return true;
}

bool RFMasterCallback::error() throw()
{
  return true;
}

bool RFMasterCallback::configure() throw()
{
  b2nsm_context(getCommunicator()->getContext());
  bool ready_all = true;
  for (NSMNodeIterator it = m_nodes.begin();
       it != m_nodes.end();) {
    NSMNode& node(it->second);
    if (node.getState() != RCState::READY_S) {
      ready_all = false;
      if (node.getState() == RCState::NOTREADY_S) {
        node.setState(RCState::LOADING_TS);
        b2nsm_sendreq(node.getName().c_str(), "RF_CONFIGURE", 0, NULL);
      }
      it++;
      if (it == m_nodes.end()) break;
      NSMNode& node_next(it->second);
      if (node.getName().find("evp_") != std::string::npos &&
          node_next.getName().find("evp_") != std::string::npos)
        continue;
      return true;
    }
    it++;
  }
  if (ready_all) {
    getNode().setState(RCState::READY_S);
    reply(true);
  }
  return true;
}

bool RFMasterCallback::unconfigure() throw()
{
  b2nsm_context(getCommunicator()->getContext());
  bool notready_all = true;
  for (NSMNodeRIterator it = m_nodes.rbegin();
       it != m_nodes.rend();) {
    NSMNode& node(it->second);
    if (node.getState() != RCState::NOTREADY_S) {
      notready_all = false;
      if (node.getState() == RCState::READY_S) {
        node.setState(RCState::ABORTING_RS);
        b2nsm_sendreq(node.getName().c_str(), "RF_UNCONFIGURE", 0, NULL);
      }
      it++;
      if (it == m_nodes.rend()) break;
      NSMNode& node_next(it->second);
      if (node.getName().find("evp_") != std::string::npos &&
          node_next.getName().find("evp_") != std::string::npos)
        continue;
      return true;
    }
    it++;
  }
  if (notready_all) {
    getNode().setState(RCState::NOTREADY_S);
    reply(true);
  }
  return true;
}

bool RFMasterCallback::start() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->Start(msg, nsmc);
  getNode().setState(RCState::RUNNING_S);
  return true;
}

bool RFMasterCallback::stop() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->Stop(msg, nsmc);
  getNode().setState(RCState::READY_S);
  return true;
}

bool RFMasterCallback::pause() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->Pause(msg, nsmc);
  getNode().setState(RCState::PAUSED_S);
  return true;
}

bool RFMasterCallback::resume() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->Resume(msg, nsmc);
  getNode().setState(RCState::RUNNING_S);
  return true;
}

bool RFMasterCallback::restart() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->Restart(msg, nsmc);
  getNode().setState(RCState::READY_S);
  return true;
}

bool RFMasterCallback::status() throw()
{
  NSMmsg* msg = getCommunicator()->getMessage().getMsg();
  NSMcontext* nsmc =  getCommunicator()->getContext();
  b2nsm_context(nsmc);
  m_master->Status(msg, nsmc);
  return true;
}


