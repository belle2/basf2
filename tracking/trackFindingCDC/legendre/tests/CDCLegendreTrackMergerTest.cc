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
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackMerger.h>

#include <vector>
#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

std::pair<TrackCandidate*, TrackCandidate*> createSplittedTracks(TrackCandidate* fullTrackCandidate)
{

  TrackCandidate* trackPartOne = new TrackCandidate(*fullTrackCandidate);
  TrackCandidate* trackPartTwo = new TrackCandidate(*fullTrackCandidate);

  // Delete the first half of hits of track one and the second half of track two
  std::vector<TrackHit*>& trackHitsOne = trackPartOne->getTrackHits();
  std::vector<TrackHit*>& trackHitsTwo = trackPartTwo->getTrackHits();

  // we do not have to delete the hits here, because the TearDown will keep track of them anyway
  trackHitsOne.resize(trackHitsOne.size() / 2);
  auto last_element = std::find(trackHitsTwo.begin(), trackHitsTwo.end(), trackHitsOne[trackHitsOne.size() - 1]);
  std::rotate(trackHitsTwo.begin(), last_element, trackHitsTwo.end());
  trackHitsTwo.resize(trackHitsTwo.size() / 2);

  EXPECT_EQ(trackHitsOne.size(), trackHitsTwo.size());

  return std::make_pair(trackPartOne, trackPartTwo);
}

TEST_F(CDCLegendreTestFixture, legendre_trackMergerTestOne)
{
  std::list<TrackCandidate*> trackList;

  TrackCandidate* fullTrackCandidate = getTrackCandidate(0);

  std::pair<TrackCandidate*, TrackCandidate*> splittedTracks = createSplittedTracks(fullTrackCandidate);

  TrackCandidate* trackPartOne = splittedTracks.first;
  TrackCandidate* trackPartTwo = splittedTracks.second;

  trackList.push_back(trackPartOne);
  trackList.push_back(trackPartTwo);

  unsigned int sizeBefore = trackPartOne->getNHits() + trackPartTwo->getNHits();

  TrackMerger::tryToMergeTrackWithOtherTracks(trackPartOne, trackList);

  // we do only want one track after merging
  EXPECT_EQ(trackList.size(), 1);
  // we do want nearly as many hits as before
  EXPECT_GT((*trackList.begin())->getNHits(), sizeBefore - 5);

  delete trackPartOne;
  delete trackPartTwo;
}

TEST_F(CDCLegendreTestFixture, legendre_trackMergerTestAll)
{
  std::list<TrackCandidate*> trackList;

  TrackCandidate* fullTrackCandidate = getTrackCandidate(0);

  std::pair<TrackCandidate*, TrackCandidate*> splittedTracks = createSplittedTracks(fullTrackCandidate);

  TrackCandidate* trackPartOne = splittedTracks.first;
  TrackCandidate* trackPartTwo = splittedTracks.second;

  trackList.push_back(trackPartOne);
  trackList.push_back(trackPartTwo);

  unsigned int sizeBefore = trackPartOne->getNHits() + trackPartTwo->getNHits();

  TrackMerger::doTracksMerging(trackList);

  // we do only want one track after merging
  EXPECT_EQ(trackList.size(), 1);
  // we do want nearly as many hits as before
  EXPECT_GT((*trackList.begin())->getNHits(), sizeBefore - 5);

  delete trackPartOne;
  delete trackPartTwo;
}
