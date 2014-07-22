#include "daq/slc/runcontrol/RunSummary.h"

using namespace Belle2;

RunSummary::RunSummary(const NSMNode& node)
  : m_node(node)
{
  setNode(node.getName());
  setConfig(false);
  setTable("runsummary");
  setRevision(1);
  addInt("runsetting", 0);
  addText("state", "NOTREADY");
  addText("cause", "manual");//"manual,auto,runend,error,error_manual"
}

RunSummary::~RunSummary() throw()
{
}

void RunSummary::setState(const NSMNode& node)
{
  setText("state", node.getState().getLabel());
}

void RunSummary::setCause(Cause cause)
{
  switch (cause) {
    case MANUAL: setText("cause", "manual"); break;
    case AUTO: setText("cause", "auto"); break;
    case RUNEND: setText("cause", "runend"); break;
    case ERROR: setText("cause", "error"); break;
    case ERROR_MANUAL: setText("cause", "error_manual"); break;
  }
}

void RunSummary::setRunSetting(const RunSetting& obj)
{
  setInt("runsetting", obj.getId());
}

void RunSummary::setNodeState(const NSMNodeList& node_v)
{
  ConfigObjectList obj_v;
  for (size_t i = 0; i < node_v.size(); i++) {
    ConfigObject obj;
    obj.setNode(m_node.getName());
    obj.setConfig(false);
    obj.setTable("runsummary.node");
    obj.setRevision(1);
    obj.setIndex(i);
    obj.addText("state", node_v[i].getState().getLabel());
    obj_v.push_back(obj);
  }
  addObjects("node", obj_v);
}

void RunSummary::setNodeData(const NSMDataList& data_v)
{
  ConfigObjectList obj_v;
  for (size_t i = 0; i < data_v.size(); i++) {
    ConfigObject obj;
    obj.setNode(m_node.getName());
    obj.setConfig(false);
    obj.setTable("runsummary.data");
    obj.setRevision(1);
    obj.setIndex(i);
    obj.addInt("id", data_v[i].getId());
    obj_v.push_back(obj);
  }
  addObjects("data", obj_v);
}
