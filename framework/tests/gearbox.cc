#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/LogSystem.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <queue>
#include <cstdlib>

#include <boost/foreach.hpp>

using namespace std;

namespace Belle2 {

  /** Test opening Belle2.xml and accessing nodes from Gearbox */
  TEST(GearBox, Open)
  {
    //LogSystem::Instance().getLogConfig()->setLogLevel(LogConfig::c_Debug);
    //LogSystem::Instance().getLogConfig()->setDebugLevel(1000);

    Gearbox& gb = Gearbox::getInstance();
    vector<string> backends;
    //Environment::getDataSearchPath useless here since it gets set only in basf2
    string local_dir(getenv("BELLE2_LOCAL_DIR"));
    backends.push_back("file:" + local_dir + "/data");
    gb.setBackends(backends);
    gb.open("geometry/Belle2.xml");
    queue<GearDir> nodes;
    nodes.push(GearDir());
    while (!nodes.empty()) {
      GearDir node = nodes.front();
      nodes.pop();
      BOOST_FOREACH(const GearDir & child, node.getNodes("*")) {
        nodes.push(child);
      }
    }
    gb.close();
  }

#ifdef HAS_SQLITE
  /** Test opening SQLite db and accessing nodes from Gearbox */
  TEST(GearBox, SQLite)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreObjPtr<EventMetaData>::registerPersistent();
    DataStore::Instance().setInitializeActive(false);
    StoreObjPtr<EventMetaData> eventMetaDataPtr;
    eventMetaDataPtr.create();
    eventMetaDataPtr->setExperiment(1);
    eventMetaDataPtr->setRun(1);

    Gearbox& gb = Gearbox::getInstance();
    vector<string> backends;
    string local_dir(getenv("BELLE2_LOCAL_DIR"));
    backends.push_back("sqlite:" + local_dir + "/framework/tests/gearbox.sqlite");
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
