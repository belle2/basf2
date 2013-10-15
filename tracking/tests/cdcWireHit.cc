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


namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Test class for the WireHitCreator object.
   */
  class CDCWireHitTest : public ::testing::Test {
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
  TEST_F(CDCWireHitTest, isEqual)
  {
    CDCHit hit1(3, 5, 1, 1, 1);
    CDCLocalTracking::CDCWireHit cdcWireHit1(&hit1, 1);
    CDCHit hit2(3, 5, 1, 1, 2);
    CDCLocalTracking::CDCWireHit cdcWireHit2(&hit2, 1);

    EXPECT_FALSE(CDCLocalTracking::CDCWireHit::ptrIsEqual(&cdcWireHit1, &cdcWireHit2));
//   B2ERROR("Empty test");
  }

  TEST_F(CDCWireHitTest, getWire)
  {
    CDCHit hit1(3, 5, 1, 1, 1);
    CDCLocalTracking::CDCWireHit cdcWireHit1(&hit1, 1);
    CDCHit hit2(3, 5, 1, 1, 1);
    CDCLocalTracking::CDCWireHit cdcWireHit2(&hit2, 1);

    EXPECT_TRUE(cdcWireHit1.getWire() == cdcWireHit2.getWire());
  }
}  // namespace

