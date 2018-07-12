#include <gtest/gtest.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/database/Database.h>
#include <framework/database/LocalDatabase.h>
#include <framework/utilities/FileSystem.h>

#include <trg/cdc/NeuroTrigger.h>
#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/BinnedEventT0.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <iostream>

using namespace Belle2;
using namespace CDC;
using namespace std;

namespace TrgTest {

  /** Test class for the NeuroTrigger */
  class NeuroTriggerTest : public TestHelpers::TestWithGearbox {
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
      StoreArray<CDCTriggerTrack> tracks;
      tracks.registerInDataStore();
      StoreObjPtr<BinnedEventT0> eventT0;
      eventT0.registerInDataStore();
      tracks.registerRelationTo(segmentHits);
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


  /** Test the Neurotrigger hit pattern selection */
  TEST_F(NeuroTriggerTest, HitPattern)
  {
    // 2 test MLPs
    NeuroTrigger neuroTrigger;
    NeuroTrigger::Parameters p;
    p.nMLP = 2;
    p.maxHitsPerSL = {1, 2};
    // binary patterns:
    // sector 1: pattern = 001111111, mask = 010000001
    //  - require SL0
    //  - use SL2 to SL6 if present
    //  - require that SL7 is missing
    //  - ignore SL8)
    // sector 2: pattern = 000000111 000111111, mask = 001001001 011011011
    //  - require 2 hits in SL0
    //  - require at least 1 hit in SL1, use up to 2
    //  - use up to 2 hits in SL2
    //  - require exactly 1 hit in SL3
    //  - require at least 1 hit in SL4, use only 1
    //  - use up to 1 hit in SL5
    //  - require that SL6 and SL7 are missing
    //  - ignore SL8
    p.SLpattern = {127, 3647};
    p.SLpatternMask = {129, 37595};
    neuroTrigger.initialize(p);

    // test track: phi = 0, no curvature
    StoreArray<CDCTriggerTrack> tracks("CDCTriggerTracks");
    CDCTriggerTrack* track = tracks.appendNew();
    // list of test hits: 3 for each super layer with identical id and priority, but different LR and time
    vector<vector<CDCTriggerSegmentHit>> tsHitList;
    unsigned nTS = 0;
    CDCGeometryPar& cdc = CDCGeometryPar::Instance();
    for (unsigned iSL = 0; iSL < 9; ++iSL) {
      unsigned iL = (iSL > 0) ? 2 : 3;
      WireID iW(iSL, iL, 0);
      vector<CDCTriggerSegmentHit> tsHitListSL;
      for (unsigned ihit = 0; ihit < 4; ++ihit) {
        short tdc = ihit * 10;
        unsigned short LR = 3 - ihit;
        tsHitListSL.push_back(CDCTriggerSegmentHit(CDCHit(tdc, 0, iW), nTS, 3, LR, tdc, 0, 0));
      }
      tsHitList.push_back(tsHitListSL);
      nTS += cdc.nWiresInLayer(iW.getICLayer());
    }

    // dummy event time
    StoreObjPtr<BinnedEventT0> T0;
    T0.create();
    T0->addBinnedEventT0(0, Const::CDC);
    neuroTrigger.getEventTime(0, *track);

    // define different hit numbers to test
    // the basis is a combination that should pass all requirements,
    // followed by individually varying all entries from 0 to 2
    vector<vector<unsigned>> nHitsPerSL = {{2, 1, 0, 1, 1, 0, 0, 0, 0}};
    for (unsigned iSL = 0; iSL < 9; ++iSL) {
      for (unsigned n = 0; n < tsHitList[iSL].size() - 1; ++n) {
        vector<unsigned> nHits = nHitsPerSL[0];
        if (nHits[iSL] != n) {
          nHits[iSL] = n;
          nHitsPerSL.push_back(nHits);
        }
      }
    }
    // the last test case has 3 hits to test the hit selection
    // (only for layers that have no maximum hit requirement)
    nHitsPerSL.push_back({3, 3, 3, 1, 3, 3, 0, 0, 3});

    // define expected results
    vector<vector<bool>> expectedUseForTrain(2, vector<bool>(nHitsPerSL.size(), true));
    expectedUseForTrain[0][1] = false;  // SL0 missing
    expectedUseForTrain[1][1] = false;  // SL0 missing
    expectedUseForTrain[1][2] = false;  // SL0 2nd hit missing
    expectedUseForTrain[1][3] = false;  // SL1 missing
    expectedUseForTrain[1][7] = false;  // SL3 missing
    expectedUseForTrain[1][9] = false;  // SL4 missing
    vector<vector<bool>> expectedUseForTest = expectedUseForTrain;
    expectedUseForTest[0][15] = false;  // 1 hit in SL7
    expectedUseForTest[0][16] = false;  // 2 hits in SL7
    expectedUseForTest[1][8] = false;  // 2 hits in SL3
    expectedUseForTest[1][13] = false;  // 1 hit in SL6
    expectedUseForTest[1][14] = false;  // 2 hits in SL6
    expectedUseForTest[1][15] = false;  // 1 hit in SL7
    expectedUseForTest[1][16] = false;  // 2 hits in SL7
    vector<vector<unsigned>> expectedMaxHitsPerSL {{1, 1, 1, 1, 1, 1, 1, 0, 0}, {2, 2, 2, 1, 1, 1, 0, 0, 0}};
    // expected result for the last test case with 3 hits per super layer
    // the index refers to tsHits, the order does not have to match
    vector<vector<unsigned>> expectedHitSelection {{1, 4, 7, 9, 11, 14}, {1, 2, 4, 5, 7, 8, 9, 11, 14}};

    StoreArray<CDCTriggerSegmentHit> tsHits("CDCTriggerSegmentHits");
    for (unsigned i = 0; i < nHitsPerSL.size(); ++i) {
      // fill hit list
      tsHits.clear();
      for (unsigned iSL = 0; iSL < 9; ++iSL) {
        for (unsigned ihit = 0; ihit < nHitsPerSL[i][iSL]; ++ihit) {
          CDCTriggerSegmentHit* hit = tsHits.appendNew(tsHitList[iSL][ihit]);
          track->addRelationTo(hit);
        }
      }
      for (unsigned isector = 0; isector < p.nMLP; ++isector) {
        // check hit pattern
        unsigned long hitPattern = neuroTrigger.getInputPattern(isector, *track);
        unsigned long sectorPattern = neuroTrigger[isector].getSLpattern();
        bool useForTrain = ((hitPattern & sectorPattern) == sectorPattern);
        bool useForTest = (hitPattern == sectorPattern);
        EXPECT_EQ(expectedUseForTrain[isector][i], useForTrain) << "sector " << isector << " hitPattern " << hitPattern;
        EXPECT_EQ(expectedUseForTest[isector][i], useForTest) << "sector " << isector << " hitPattern " << hitPattern;
        // check number of selected hits
        vector<unsigned> hitIndices = neuroTrigger.selectHits(isector, *track);
        vector<unsigned> selectedHitsPerSL(9, 0);
        for (unsigned ii = 0; ii < hitIndices.size(); ++ii) {
          ++selectedHitsPerSL[tsHits[hitIndices[ii]]->getISuperLayer()];
        }
        for (unsigned iSL = 0; iSL < 9; ++iSL) {
          EXPECT_LE(selectedHitsPerSL[iSL], expectedMaxHitsPerSL[isector][iSL]) << "sector " << isector << " SL " << iSL;
          if (nHitsPerSL[i][iSL] <= expectedMaxHitsPerSL[isector][iSL]) {
            EXPECT_EQ(selectedHitsPerSL[iSL], nHitsPerSL[i][iSL]) << "sector " << isector << " SL " << iSL;
          }
        }
        // check index of selected hits
        if (i == nHitsPerSL.size() - 1) {
          EXPECT_EQ(expectedHitSelection[isector].size(), hitIndices.size()) << "sector " << isector;
          sort(hitIndices.begin(), hitIndices.end());
          for (unsigned ii = 0; ii < hitIndices.size() && ii < expectedHitSelection[isector].size(); ++ii) {
            EXPECT_EQ(expectedHitSelection[isector][ii], hitIndices[ii]) << "sector " << isector << " ii " << ii;
          }
        }
      }
    }
  }
}
