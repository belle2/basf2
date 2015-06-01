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

#ifdef HAS_SQLITE
  /** Test opening SQLite db and accessing nodes from Gearbox */
  TEST(GearBox, SQLite)
  {
    StoreObjPtr<EventMetaData> eventMetaDataPtr;
    DataStore::Instance().setInitializeActive(true);
    eventMetaDataPtr.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    eventMetaDataPtr.create();
    eventMetaDataPtr->setExperiment(1);
    eventMetaDataPtr->setRun(1);

    Gearbox& gb = Gearbox::getInstance();
    vector<string> backends;
    backends.push_back("sqlite:" + FileSystem::findFile("/framework/tests/gearbox.sqlite"));
    gb.setBackends(backends);
    gb.open("Belle2.xml");
    EXPECT_EQ(3, gb.getNumberNodes("/detector/*"));
    EXPECT_EQ(125.0, gb.getDouble("/detector/foo"));
    EXPECT_EQ(12.50, gb.getLength("/detector/foo"));
    EXPECT_EQ(0.5, gb.getDouble("/detector/bar"));
    EXPECT_EQ(0.5, gb.getLength("/detector/bar"));
    gb.close();

    DataStore::Instance().reset(DataStore::c_Event);
  }
#endif


}  // namespace
