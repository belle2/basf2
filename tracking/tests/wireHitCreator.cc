#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <cdc/dataobjects/CDCHit.h>
#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>
#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>
#include <tracking/cdcLocalTracking/creators/WireHitCreator.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <gtest/gtest.h>

using namespace std;

//using namespace CDCLocalTracking;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Test class for the WireHitCreator object.
   */
  class WireHitCreatorTest : public ::testing::Test {
  protected:

    virtual void SetUp() {
      //Set up geometry for using CDCHit class
      Gearbox& gb = Gearbox::getInstance();
      vector<string> backends;
      backends.push_back("file:");
      gb.setBackends(backends);
      gb.open("geometry/Belle2.xml");
    }

  };


  /** Test simple Setters and Getters.
   */
  TEST_F(WireHitCreatorTest, create)
  {
    //Create CDCHit and fill CDCWireHitCollection with it
    DataStore::Instance().setInitializeActive(true);
    StoreArray<CDCHit>::registerPersistent();
    StoreArray<CDCHit> cdcHits;

//    CDCHit hit1(3, 5, 0, 15, 20);
    CDCHit hit1(3, 5, 1, 1, 1);
    new(cdcHits.nextFreeAddress()) CDCHit(hit1);
    CDCLocalTracking::WireHitCreator wireHitCreator;
    CDCLocalTracking::CDCWireHitCollection cdcWireHitCollection;
    wireHitCreator.create(cdcHits, cdcWireHitCollection);

    EXPECT_EQ(cdcWireHitCollection.size(), 1);
//    B2ERROR("Test disabled until solution for Gearbox mock up");
  }
}  // namespace
