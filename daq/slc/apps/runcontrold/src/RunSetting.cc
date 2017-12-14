#include "daq/slc/apps/runcontrold/RunSetting.h"

using namespace Belle2;

RunSetting::RunSetting(const NSMNode& /*node*/)
{
  addInt("runnumberid", 0);
  //addInt("runcontrol", 0);
  addText("operators", "");
  addText("comment", "");
}

RunSetting::~RunSetting() throw()
{
}

void RunSetting::setRunNumber(const RunNumber& info)
{
  setInt("runnumberid", info.getId());
}

void RunSetting::setRunControl(const DBObject& obj)
{
  //setInt("runcontrol", obj.getId());
  addObject("runcontrol", obj);
}
