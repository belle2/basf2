#include <gtest/gtest.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/database/Database.h>
#include <framework/database/LocalDatabase.h>
#include <framework/utilities/FileSystem.h>

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <iostream>

using namespace Belle2;
using namespace CDC;
using namespace std;

namespace TrgTest {

  /** Test class for the Track Segment Finder */
  class TSFTest : public TestHelpers::TestWithGearbox {
  protected:
    /** set up the test environment */
    virtual void SetUp()
    {
      // Stolen from
      // tracking/trackFindingCDC/testFixtures/src/TackFindingCDCTestWithTopology.cc,
      // Oliver <oliver.frost@desy.de> should know the details if this stops working ;)

      StoreObjPtr<EventMetaData> evtPtr;
      DataStore::Instance().setInitializeActive(true);
      evtPtr.registerInDataStore();
      StoreArray<CDCTriggerSegmentHit> segmentHits;
      segmentHits.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
      evtPtr.construct(0, 0, 1);

      // Setup the Database
      Database::reset();
      // Load only the local database
      std::string dbFilePath = FileSystem::findFile("data/framework/database.txt");
      LocalDatabase::createInstance(dbFilePath, "", true, LogConfig::c_Error);
      DBStore::Instance().update();
      // Unpack the cdc geometry, which currently uses both the db and the gearbox
      GearDir cdcGearDir = Gearbox::getInstance().getDetectorComponent("CDC");
      CDCGeometry cdcGeometry;
      cdcGeometry.read(cdcGearDir);
      CDC::CDCGeometryPar::Instance(&cdcGeometry);
    }

    /** Close the gearbox and reset the global objects */
    virtual void TearDown()
    {
      Database::reset();
      DataStore::Instance().reset();
    }
  };


  /** Test the consistency of the two TS hit constructors
   *  (by continuous ID or by local ID and super layer) */
  TEST_F(TSFTest, SegmentID)
  {
    CDCGeometryPar& cdc = CDCGeometryPar::Instance();
    unsigned nTS = 0;
    // check all super layers
    for (unsigned iSL = 0; iSL < 9; ++iSL) {
      unsigned nWires = cdc.nWiresInLayer(WireID(iSL, 0, 0).getICLayer());
      // check at beginning, middle and end of layer
      std::vector<unsigned> wireIDs = {0, nWires / 2, nWires - 1};
      for (unsigned iWire : wireIDs) {
        // check all priority positions
        for (unsigned short priority = 1; priority < 4; ++priority) {
          unsigned short segmentID = nTS + iWire;
          // define hits
          CDCTriggerSegmentHit hitGlobal =
            CDCTriggerSegmentHit(segmentID, priority, 0, 0, 0, 0);
          CDCTriggerSegmentHit hitLocal =
            CDCTriggerSegmentHit(iSL, iWire, priority, 0, 0, 0, 0);
          // compare super layer and global segment ID
          EXPECT_EQ(hitGlobal.getISuperLayer(), iSL)
              << "iSL " << iSL << " iWire " << iWire
              << " priority " << priority << " segmentID " << segmentID;
          EXPECT_EQ(hitLocal.getSegmentID(), segmentID)
              << "iSL " << iSL << " iWire " << iWire
              << " priority " << priority << " segmentID " << segmentID;
          // compare layer and wire ID of the two hits to each other
          EXPECT_EQ(hitGlobal.getILayer(), hitLocal.getILayer())
              << "iSL " << iSL << " iWire " << iWire
              << " priority " << priority << " segmentID " << segmentID;
          EXPECT_EQ(hitGlobal.getIWire(), hitLocal.getIWire())
              << "iSL " << iSL << " iWire " << iWire
              << " priority " << priority << " segmentID " << segmentID;
          // check wire ID range
          EXPECT_LT(hitGlobal.getIWire(), nWires)
              << "iSL " << iSL << " iWire " << iWire
              << " priority " << priority << " segmentID " << segmentID;
          EXPECT_LT(hitLocal.getIWire(), nWires)
              << "iSL " << iSL << " iWire " << iWire
              << " priority " << priority << " segmentID " << segmentID;
        }
      }
      nTS += nWires;
    }
  }
}
