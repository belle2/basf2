#include "daq/slc/apps/flowmonitor/FlowMonitorCallback.h"

#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/apps/runcontrold/rorc_status.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/readout/ronode_status.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/Date.h>

using namespace Belle2;

FlowMonitorCallback::FlowMonitorCallback(const std::string& node, const std::string& rcnode)
  : NSMCallback(20), m_runcontrol(rcnode)
{
  setNode(NSMNode(node));
  allocData(getNode().getName() + "_STATUS", "rorc_status", rorc_status_revision);
}

void FlowMonitorCallback::init(NSMCommunicator& com) throw()
{
  try {
    int nnodes = 0;
    get(m_runcontrol, "nnodes", nnodes);
    for (int i = 0; i < nnodes; i++) {
      std::string vname = StringUtil::form("node[%d].name", i);
      std::string name;
      get(m_runcontrol, vname, name);
      if ((StringUtil::find(name, "CPR") || StringUtil::find(name, "ROPC"))) {
        LogFile::debug("add ronode : %s", name.c_str());
        m_nodes.insert(NSMNodeMap::value_type(name, NSMNode(name)));
        m_names.push_back(name);
      }
    }
    for (StringList::iterator it = m_names.begin(); it != m_names.end(); it++) {
      std::string name = *it;
      std::string dataname, format;
      int revision = -1;
      int tstamp = 0;
      NSMNode& node(m_nodes[name]);
      try {
        NSMCommunicator::connected(node.getName());
        get(node, "nsmdata.name", dataname, 2);
        try {
          getData(dataname);
          continue;
        } catch (const std::out_of_range& e) {
        }
        get(node, "nsmdata.format", format, 2);
        if (format != "ronode_status") continue;
        get(node, "nsmdata.revision", revision, 2);
        get(node, "nsmdata.tstamp", tstamp, 2);
        openData(dataname, format, revision);
        m_hostnames.push_back(node.getName());
        m_datanames.push_back(dataname);
        LogFile::info("%s:%s:%d", dataname.c_str(), format.c_str(), revision);
        std::string vname = StringUtil::form("node[%d].", (int)getDataMap().size());
        add(new NSMVHandlerText(vname + "name", true, false, name));
        add(new NSMVHandlerText(vname + "data", true, false,
                                StringUtil::form("%s:%s:%d", dataname.c_str(),
                                                 format.c_str(), revision)));
      } catch (const NSMNotConnectedException& e) {
      }
    }
  } catch (const IOException& e) {
    LogFile::warning(e.what());
  }
}

void FlowMonitorCallback::vset(NSMCommunicator& com, const NSMVar& var) throw()
{
  if (var.getName() == "nsmdata.tstamp" && var.getType() == NSMVar::INT) {
    NSMMessage& msg(com.getMessage());
    rorc_status* status = (rorc_status*)getData().get();
    for (size_t i = 0; i < m_hostnames.size(); i++) {
      if (m_hostnames[i] == msg.getNodeName()) {
        NSMData& data(getData(m_datanames[i]));
        try {
          if (!data.isAvailable()) data.open(com);
        } catch (const NSMHandlerException& e) {
          continue;
        }
        NSMNode node(msg.getNodeName());
        get(node, data);
        ronode_status* ronode = (ronode_status*)data.get();
        std::string name = StringUtil::tolower(data.getName()).c_str();
        LogFile::debug("%s.ctime : %s", name.c_str(),
                       Date(ronode->ctime).toString());
        memcpy(&status->ro[i], ronode, sizeof(ronode_status));
      }
    }
  } else {
    NSMCallback::vset(com, var);
  }
}

void FlowMonitorCallback::timeout(NSMCommunicator&) throw()
{
}

