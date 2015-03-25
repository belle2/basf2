/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackProcessor.h>
#include <tracking/trackFindingCDC/legendre/tests_fixtures/CDCLegendreTestFixture.h>

#include <vector>
#include <gtest/gtest.h>

#include <genfit/TrackCand.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

class CDCLegendreTrackProcessor : public CDCLegendreTestFixture  {
protected:
  void SetUp() override
  {
    CDCLegendreTestFixture::SetUp();

    StoreArray<genfit::TrackCand> trackCands(m_track_cands_store_array_name);
    DataStore::Instance().setInitializeActive(true);
    trackCands.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);
  }

  void TearDown() override
  {
    DataStore::Instance().reset();
  }

  const std::string m_track_cands_store_array_name = "GFTrackCands";
};

TEST_F(CDCLegendreTrackProcessor, legendre_createGFTracks)
{
  TrackProcessor trackProcessor;
  list<TrackCandidate*>& trackList = trackProcessor.getTrackList();

  double theta_test = 1.4;
  double r_test = 5;
  int charge_test = 1;
  TVector3 reference_test(3, 4, 0);

  // We only need the hits from this candidate. We set the parameters by ourself.
  TrackCandidate* testCandidate = getTrackCandidate(0);
  testCandidate->setR(r_test);
  testCandidate->setTheta(theta_test);
  testCandidate->setCharge(charge_test);
  testCandidate->setReferencePoint(reference_test.X(), reference_test.Y());
  trackList.push_back(testCandidate);


  trackProcessor.createGFTrackCandidates(m_track_cands_store_array_name);

  double expected_pt = 1.5 * 0.00299792458 / fabs(r_test);

  StoreArray<genfit::TrackCand> trackCands(m_track_cands_store_array_name);
  genfit::TrackCand* resultedTrackCand = trackCands[0];
  EXPECT_EQ(resultedTrackCand->getPosSeed().X(), reference_test.X());
  EXPECT_EQ(resultedTrackCand->getPosSeed().Y(), reference_test.Y());
  EXPECT_EQ(resultedTrackCand->getMomSeed().Pt(), expected_pt);

  delete testCandidate;
}
