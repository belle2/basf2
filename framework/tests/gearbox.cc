#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/utilities/FileSystem.h>

#include <gtest/gtest.h>
#include <string>
#include <queue>

using namespace std;
using namespace Belle2;

namespace {

  /** Test opening xml file and accessing nodes from Gearbox */
  TEST(GearBox, Open)
  {
    Gearbox& gb = Gearbox::getInstance();
    vector<string> backends;
    backends.push_back("file:");
    gb.setBackends(backends);
    gb.open("geometry/SuperKEKB.xml");
    queue<GearDir> nodes;
    nodes.push(GearDir());
    while (!nodes.empty()) {
      GearDir node = nodes.front();
      nodes.pop();
      for (const GearDir& child : node.getNodes("*")) {
        nodes.push(child);
      }
    }
    gb.close();
  }
}  // namespace
