#include "base/RunStatus.h"

#include <cstring>

using namespace Belle2;

RunStatus::RunStatus(const std::string& data_name, int revision) throw()
  : DataObject(data_name, "RunStatus")
{
  setRevision(revision);
  addInt("run_number", 0);
  addInt("exp_number", 0);
  addInt("start_time", 0);
  addInt("end_time", -1);
}

