/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/tests_fixtures/CDCLegendreTestFixture.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreSimpleFilter.h>

#include <vector>
#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

/*
 * For testing the track candidate we run into the (good) issue, that we can not add totally wrong hits to a track candidate (internal checks).
 * This is why we first extract the pure vector of hits and add the track hits to this vector.
*/

TEST_F(CDCLegendreTestFixture, legendre_SimpleFilterAssignmentProbabilityTest)
{
  TrackCandidate* trackCandidate = getTrackCandidate(0);
  TrackCandidate* otherTrackCandidate = getTrackCandidate(1);

  // Test the assignment probability
  for (TrackHit* trackHit : trackCandidate->getTrackHits()) {
    EXPECT_GT(SimpleFilter::getAssigmentProbability(trackHit, trackCandidate), 0.5);
  }

  for (TrackHit* trackHit : otherTrackCandidate->getTrackHits()) {
    EXPECT_GT(SimpleFilter::getAssigmentProbability(trackHit, otherTrackCandidate), 0.5);
  }


  unsigned int numberOfNearHits = 0;
  for (TrackHit* trackHit : otherTrackCandidate->getTrackHits()) {
    if (SimpleFilter::getAssigmentProbability(trackHit, trackCandidate) > 0.5)
      numberOfNearHits++;
  }
  EXPECT_LT(numberOfNearHits, otherTrackCandidate->getNHits() / 10);

  numberOfNearHits = 0;
  for (TrackHit* trackHit : trackCandidate->getTrackHits()) {
    if (SimpleFilter::getAssigmentProbability(trackHit, otherTrackCandidate) > 0.5)
      numberOfNearHits++;
  }
  EXPECT_LT(numberOfNearHits, trackCandidate->getNHits() / 10);
}

TEST_F(CDCLegendreTestFixture, legendre_SimpleFilterAppendUnusedHitsTest)
{
  TrackCandidate* trackCandidate = getTrackCandidate(0);
  TrackCandidate* otherTrackCandidate = getTrackCandidate(1);

  std::vector<TrackHit*> someHits = getSomeHits(otherTrackCandidate);

  std::list<TrackCandidate*> trackList;
  trackList.push_back(trackCandidate);
  trackList.push_back(otherTrackCandidate);

  unsigned int numberOfHitsBefore = trackCandidate->getNHits();
  unsigned int numberOfHitsBeforeOther = otherTrackCandidate->getNHits();
  SimpleFilter::appendUnusedHits(trackList, someHits);
  EXPECT_EQ(trackCandidate->getNHits(), numberOfHitsBefore) << "Added hits to the wrong track!";
  EXPECT_EQ(otherTrackCandidate->getNHits(), numberOfHitsBeforeOther + someHits.size()) << "Added too few hits to the track.";
}

TEST_F(CDCLegendreTestFixture, legendre_SimpleFilterDeleteAllMarkedHitsTest)
{
  TrackCandidate* trackCandidate = getTrackCandidate(0);

  for (TrackHit* trackHit : trackCandidate->getTrackHits()) {
    trackHit->setHitUsage(TrackHit::bad);
  }

  SimpleFilter::deleteAllMarkedHits(trackCandidate);
  EXPECT_EQ(0, trackCandidate->getNHits());
}

TEST_F(CDCLegendreTestFixture, legendre_SimpleFilterDeleteWrongHitsOfTrackTest)
{
  TrackCandidate* trackCandidate = getTrackCandidate(0);
  TrackCandidate* otherTrackCandidate = getTrackCandidate(1);

  std::vector<TrackHit*> someHits = getSomeHits(otherTrackCandidate);

  unsigned int numberOfHitsBefore = trackCandidate->getNHits();

  std::vector<TrackHit*>& trackHitList = trackCandidate->getTrackHits();
  for (TrackHit* trackHit : someHits) {
    trackHitList.push_back(trackHit);
  }

  EXPECT_EQ(trackCandidate->getNHits(), numberOfHitsBefore + someHits.size());

  SimpleFilter::deleteWrongHitsOfTrack(trackCandidate, 0.8);
  EXPECT_EQ(trackCandidate->getNHits(), numberOfHitsBefore);
}

TEST_F(CDCLegendreTestFixture, legendre_SimpleFilterReassignHitsFromOtherTracksTest)
{
  TrackCandidate* trackCandidate = getTrackCandidate(0);
  TrackCandidate* otherTrackCandidate = getTrackCandidate(1);

  std::vector<TrackHit*> someHitsFromOther = getSomeHits(otherTrackCandidate);
  std::vector<TrackHit*> someHits = getSomeHits(trackCandidate);

  unsigned int numberOfHitsBefore = trackCandidate->getNHits();
  unsigned int numberOfHitsBeforeOther = otherTrackCandidate->getNHits();

  std::vector<TrackHit*>& trackHitList = trackCandidate->getTrackHits();
  for (TrackHit* trackHit : someHitsFromOther) {
    trackHitList.push_back(trackHit);
  }
  EXPECT_EQ(trackCandidate->getNHits(), numberOfHitsBefore + someHitsFromOther.size());

  std::vector<TrackHit*>& otherTrackHitList = otherTrackCandidate->getTrackHits();
  for (TrackHit* trackHit : someHits) {
    otherTrackHitList.push_back(trackHit);
  }
  EXPECT_EQ(otherTrackCandidate->getNHits(), numberOfHitsBeforeOther + someHits.size());

  std::list<TrackCandidate*> trackList;
  trackList.push_back(trackCandidate);
  trackList.push_back(otherTrackCandidate);

  SimpleFilter::reassignHitsFromOtherTracks(trackList);

  EXPECT_EQ(trackCandidate->getNHits(), numberOfHitsBefore + someHits.size());
  EXPECT_EQ(otherTrackCandidate->getNHits(), numberOfHitsBeforeOther +  someHitsFromOther.size());
}
