#include "COPPERCallback.h"

#include <nsm/NSMNodeDaemon.h>

#include <xml/ObjectLoader.h>

#include <base/Debugger.h>
#include <base/StringUtil.h>
#include <base/ConfigFile.h>

using namespace Belle2;

typedef void* func_t(void*, const char*);

int main(int argc, char** argv)
{
  if (argc < 3) {
    Belle2::debug("Usage : ./cprcontrold <name> <class_nam>");
    return 1;
  }
  const char* name = argv[1];
  const char* class_name = argv[2];
  ConfigFile config("slc_config.conf");
  ObjectLoader loader(config.get("RC_XML_PATH"));
  NSMNode* node = new NSMNode(name, loader.load(class_name));
  COPPERCallback* callback = new COPPERCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback);
  daemon->run();

  return 0;
}
