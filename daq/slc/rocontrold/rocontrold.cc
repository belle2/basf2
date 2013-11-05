#include "ROCallback.h"

#include <nsm/NSMNodeDaemon.h>

#include <xml/ObjectLoader.h>

#include <base/Debugger.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("Usage : ./rocontrold <name>");
    return 1;
  }
  const char* name = argv[1];
  const char* class_name = "ROPC";
  ObjectLoader loader(getenv("B2SC_XML_PATH"));
  NSMNode* node = new NSMNode(name, loader.load(class_name));
  ROCallback* callback = new ROCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(node, callback);
  node->setState(State::INITIAL_S);
  daemon->run();

  return 0;
}
