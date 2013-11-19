#include "base/RunConfig.h"

using namespace Belle2;

RunConfig::RunConfig(const std::string& data_name, int revision) throw()
  : DataObject(data_name, "RunConfig")
{
  setRevision(revision);
  addInt("run_number", 0);
  addInt("exp_number", 0);
  addInt("version", 0);
  addInt("start_time", 0);
  addText("run_type", "TESTT:DEFAULT", 64);
  addText("operators", ":", 64);
}

