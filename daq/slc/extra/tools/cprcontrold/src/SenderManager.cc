#include "SenderManager.h"

#include "daq/slc/system/Executor.h"

#include "daq/slc/base/StringUtil.h"

#include <cstdlib>

using namespace Belle2;

void SenderManager::run() throw()
{
  const char* belle2_path = getenv("BELLE2_LOCAL_DIR");
  const char* belle2_sub = getenv("BELLE2_SUBDIR");
  DataObject* obj = _node->getData();
  int flag = 0;
  for (size_t slot = 0; slot < 4; slot++) {
    if (obj->getBool(Belle2::form("used_%d", slot)))
      flag += 1 << slot;
  }
  Executor executor;
  executor.setExecutable("%s/bin/%s/basf2", belle2_path, belle2_sub);
  executor.addArg("%s/daq/rawdata/examples/%s",
                  belle2_path, obj->getText("script").c_str());
  executor.addArg(obj->getText("host"));
  executor.addArg("%d", (int)_node->getID());
  executor.addArg("%d", flag);
  executor.addArg("1");
  executor.addArg("--no-stats");
  executor.execute();
}

