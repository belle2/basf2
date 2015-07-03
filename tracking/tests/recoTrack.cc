/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/RecoTrack.h>

#include <cdc/dataobjects/WireID.h>
#include <cdc/dataobjects/CDCHit.h>

#include <svd/dataobjects/SVDCluster.h>

#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

#include <framework/datastore/StoreArray.h>

#include <gtest/gtest.h>

#include <vector>
#include <utility>

using namespace std;

namespace Belle2 {
  /** Test class for the RecoTrack object. */
  class RecoTrackTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(RecoTrackTest, settersNGetters)
  {
    //--- Setup -----------------------------------------------------------------------
    DataStore::Instance().setInitializeActive(true);
    StoreArray<CDCHit>::registerPersistent();
    StoreArray<CDCHit> cdcHits;

    //"CDCHit(tdcCount, adcCount, superLayer, layer, wire)"
    //Indices range from 0-7
    cdcHits.appendNew(100, 100, 0, 0, 0);
    cdcHits.appendNew(100, 100, 2, 0, 0);
    cdcHits.appendNew(100, 100, 4, 0, 0);
    cdcHits.appendNew(100, 100, 6, 0, 0);
    cdcHits.appendNew(100, 100, 8, 0, 0);
    cdcHits.appendNew(100, 100, 1, 1, 0);
    cdcHits.appendNew(100, 100, 3, 0, 0);
    cdcHits.appendNew(100, 100, 5, 0, 0);

    vector<pair<unsigned short, short> > axialWiresVector =
    {make_pair(0, 0), make_pair(1, 0), make_pair(2, 0), make_pair(3, 0), make_pair(4, 0)};

    //--- Test of CDC Stuff -------------------------------------------------------------
    RecoTrack recoTrack;
    recoTrack.setCDCHitIndices(axialWiresVector, +1);

    //Fill the arm into the hit pattern, that has actually hits.
    recoTrack.fillHitPatternCDC(+1);
    EXPECT_EQ(recoTrack.getHitPatternCDC().hasAxialLayer(),  true);
    EXPECT_EQ(recoTrack.getHitPatternCDC().hasStereoLayer(), false);

    //Now I fill the arm into the pattern, that doesn't have any hits.
    recoTrack.fillHitPatternCDC(-1);
    EXPECT_EQ(recoTrack.getHitPatternCDC().hasAxialLayer(),  false);
    EXPECT_EQ(recoTrack.getHitPatternCDC().hasStereoLayer(), false);

    //Test, if the resetting really empties the arm.
    recoTrack.resetHitIndices(+1, Const::CDC);
    recoTrack.fillHitPatternCDC(+1);
    EXPECT_EQ(recoTrack.getHitPatternCDC().hasAxialLayer(),  false);
    EXPECT_EQ(recoTrack.getHitPatternCDC().hasStereoLayer(), false);

    recoTrack.resetHitIndices(0, Const::CDC);
    EXPECT_FALSE(recoTrack.hasCDCHits());
    recoTrack.addCDCHitIndex(make_pair(static_cast<unsigned short>(0), static_cast<short>(-1)), -1);
    EXPECT_TRUE(recoTrack.hasCDCHit(0, -1));
    EXPECT_FALSE(recoTrack.hasCDCHit(0, +1));

    //--- Now we set up some SVD and PXD Clusters and test the same stuff ---------------
  }
}
