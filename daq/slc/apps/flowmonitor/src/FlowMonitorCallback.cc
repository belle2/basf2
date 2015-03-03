#include "daq/slc/apps/flowmonitor/FlowMonitorCallback.h"

#include <daq/slc/database/DBObjectLoader.h>

using namespace Belle2;

FlowMonitorCallback::FlowMonitorCallback(const std::string& conf)
{
  DBObject obj = DBObjectLoader::load(conf);
  m_runcontrol = NSMNode(obj.getText("runcontrol"));
  const DBObjectList& o_node(obj.getObjects("node"));
  for (size_t i = 0; i < o_node.size(); i++) {
    m_node.push_back(NSMNode(o_node[i].getText("name")));
  }
  const DBObjectList& o_data(obj.getObjects("data"));
  for (size_t i = 0; i < o_data.size(); i++) {
    const DBObject& cobj(o_data[i]);
    m_data.push_back(NSMData(cobj.getText("name"),
                             cobj.getText("format"),
                             cobj.getInt("revision")));
  }
}

void FlowMonitorCallback::timeout(NSMCommunicator& com) throw()
{
  for (size_t i = 0; i < m_data.size(); i++) {
    try {
      m_data[i].open(com);
    } catch (const NSMHandlerException& e) {
    }
  }
}

