#include "daq/slc/apps/flowmonitor/FlowMonitorCallback.h"

#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/apps/runcontrold/rorc_status.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/readout/ronode_status.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/Date.h>

namespace Belle2 {

  class NSMVHandlerNSMData : public NSMVHandlerInt {
  public:
    NSMVHandlerNSMData(const std::string& name,
                       FlowMonitorCallback& callback,
                       NSMCommunicator& com, int index)
      : NSMVHandlerInt(name, false, true), m_com(com),
        m_callback(callback), m_index(index) {}
    virtual ~NSMVHandlerNSMData() throw() {}
    virtual bool handleSetInt(int)
    {
      try {
        std::string dataname, vname = StringUtil::form("node[%d].", m_index);
        m_callback.get(vname + "data", dataname);
        dataname = StringUtil::split(dataname, ':')[0];
        NSMData& data(m_callback.getData(dataname));
        if (!data.isAvailable()) data.open(m_com);
        NSMNode node(getNode());
        dataname = data.getName() + "\n" + data.getFormat();
        int pars[2] = {data.getRevision(), data.getSize()};
        NSMCommunicator::send(NSMMessage(node, NSMCommand::DATAGET, 2, pars, dataname));
      } catch (const std::exception& e) {
        LogFile::error(e.what());
        return false;
      }
      return true;
    }
  private:
    NSMCommunicator& m_com;
    FlowMonitorCallback& m_callback;
    int m_index;
  };

}

using namespace Belle2;

FlowMonitorCallback::FlowMonitorCallback(const std::string& node, const std::string& rcnode)
  : NSMCallback(20), m_runcontrol(rcnode)
{
  setNode(NSMNode(node));
  allocData(getNode().getName(), "rorc_status", rorc_status_revision);
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
    timeout(com);
  } catch (const IOException& e) {
    LogFile::warning(e.what());
  }
}

void FlowMonitorCallback::nsmdataset(NSMCommunicator& com, NSMData& data) throw()
{
  const NSMMessage& msg(com.getMessage());
  NSMCallback::nsmdataset(com, data);
  try {
    for (size_t i = 0; i < m_hostnames.size(); i++) {
      if (m_hostnames[i] == msg.getNodeName() &&
          data.getName() == m_datanames[i]) {
        ronode_status* ronode = (ronode_status*)data.get();
        std::string name = StringUtil::tolower(data.getName()).c_str();
        if (getData().isAvailable()) {
          rorc_status* status = (rorc_status*)getData().get();
          memcpy(&status->node[i], ronode, sizeof(ronode_status));
          if (m_state == RCState::RUNNING_S) {
            if (ronode->flowrate_out == 0 && ronode->flowrate_in > 0) {
              LogFile::warning("Data flow is stacked in " + name);
            }
            if (ronode->nevent_in - ronode->nevent_out > m_nevents_th) {
              //LogFile::warning("More than %d events are stacked in %s",
              //         m_nevents_th, name.c_str());
              //LogFile::warning("in: %d out: %d", status->nevent_in, status->nevent_out);
            }
          }
        }
      }
    }
  } catch (const NSMNotConnectedException& e) {
    LogFile::error(e.what());
  }
}

void FlowMonitorCallback::timeout(NSMCommunicator& com) throw()
{
  try {
    std::string state_s;
    get(m_runcontrol, "rcstate", state_s);
    m_state = RCState(state_s);
    for (StringList::iterator it = m_names.begin(); it != m_names.end(); it++) {
      std::string name = *it;
      std::string dataname, format;
      int revision = -1;
      NSMNode& node(m_nodes[name]);
      try {
        NSMCommunicator::connected(node.getName());
        get(node, "nsmdata.name", dataname, 5);
        try {
          getData(dataname);
          continue;
        } catch (const std::out_of_range& e) {
        }
        get(node, "nsmdata.format", format, 5);
        if (format != "ronode_status") continue;
        get(node, "nsmdata.revision", revision, 5);
        int index = (int)getDataMap().size();
        openData(dataname, format, revision);
        m_hostnames.push_back(node.getName());
        m_datanames.push_back(dataname);
        LogFile::info("%s:%s:%d", dataname.c_str(), format.c_str(), revision);
        std::string vname = StringUtil::form("node[%d].", index);
        add(new NSMVHandlerText(vname + "name", true, false, name));
        add(new NSMVHandlerText(vname + "data", true, false,
                                StringUtil::form("%s:%s:%d", dataname.c_str(),
                                                 format.c_str(), revision)));
        get(node, new NSMVHandlerNSMData("nsmdata.tstamp", *this, com, index), 2);
      } catch (const NSMNotConnectedException& e) {
        LogFile::error(e.what());
      }
    }
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
}
