#include "daq/slc/runcontrol/RunSetting.h"

using namespace Belle2;

RunSetting::RunSetting(const NSMNode& node)
{
  setNode(node.getName());
  setConfig(false);
  setTable("runsetting");
  setRevision(1);
  addInt("runnumberid", 0);
  addText("operators", "");
  addText("comment", "");
  addInt("runcontrol", 0);
}

RunSetting::~RunSetting() throw()
{
}

void RunSetting::setRunNumber(RunNumberInfo info)
{
  setInt("runnumberid", info.getId());
}

void RunSetting::setRunControl(const ConfigObject& obj)
{
  setInt("runcontrol", obj.getId());
}
