/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Thomas Hauth <thomas.hauth@kit.edu>        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/tests_fixtures/CDCLegendreTestFixture.h>

#include <tracking/trackFindingCDC/hough/phi0_curv/HitPhi0CurvLegendre.h>
#include <tracking/trackFindingCDC/utilities/TimeIt.h>

#include <set>
#include <vector>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TEST_F(CDCLegendreTestFixture, phi0CurvHoughTreeOnTrackHits)
{
  // Prepare the hough algorithm
  const size_t maxLevel = 13;
  const size_t phi0Divisions = 2;
  const size_t curvDivisions = 2;

  // const size_t maxLevel = 8;
  // const size_t phiDivisions = 3;
  // const size_t curvDivisions = 3;

  using TrackHitPhi0CurvQuadLegendre =
    HitPhi0CurvLegendre<RLTagged<const TrackHit*>, phi0Divisions, curvDivisions>;
  TrackHitPhi0CurvQuadLegendre trackHitPhi0CurvQuadLegendre(maxLevel);
  trackHitPhi0CurvQuadLegendre.initialize();

  // Get the hits form the test event
  markAllHitsAsUnused();
  std::set<TrackHit*>& hits_set = getHitSet();
  std::vector<TrackHit*> hitVector;

  for (TrackHit* trackHit : hits_set) {
    if (trackHit->getSuperlayerId() % 2 == 0)
      hitVector.push_back(trackHit);
  }

  // Execute the finding a couple of time to find a stable execution time.
  vector< pair<Phi0CurvBox, vector<RLTagged<const TrackHit*> > > > candidates;

  // Is this still C++? Looks like JavaScript to me :-).
  TimeItResult timeItResult = timeIt(100, true, [&]() {
    // Exclude the timing of the resource release for comparision with the legendre test.
    trackHitPhi0CurvQuadLegendre.fell();

    trackHitPhi0CurvQuadLegendre.seed(hitVector);

    const double minWeight = 30.0;
    const double maxCurv = NAN;
    candidates = trackHitPhi0CurvQuadLegendre.find(minWeight, maxCurv);
    //candidates = trackHitPhi0CurvQuadLegendre.findBest(minWeight, maxCurv);

    // B2INFO("Execution " << iExecution);
    /// Check if exactly two candidates have been found
    ASSERT_EQ(numberOfPossibleTrackCandidate, candidates.size());

    // Check for the parameters of the track candidates
    // The actual hit numbers are more than 30, but this is somewhat a lower bound
    EXPECT_GE(candidates[0].second.size(), 30);
    EXPECT_GE(candidates[1].second.size(), 30);

  });

  /// Test idiom to output statistics about the tree.
  std::size_t nNodes = 0;
  using Node = TrackHitPhi0CurvQuadLegendre::Node;
  auto countNodes = [&nNodes](Node*) -> bool {
    ++nNodes;
    return true;
  };
  trackHitPhi0CurvQuadLegendre.getTree()->walk(countNodes);
  B2INFO("Tree generated " << nNodes << " nodes");
  trackHitPhi0CurvQuadLegendre.fell();
  trackHitPhi0CurvQuadLegendre.raze();


  for (std::pair<Phi0CurvBox, std::vector<RLTagged<const TrackHit*> > >& candidate : candidates) {
    const Phi0CurvBox& phi0CurvBox = candidate.first;
    const std::vector<RLTagged<const TrackHit*> >& taggedHits = candidate.second;

    B2INFO("Candidate");
    B2INFO("size " << taggedHits.size());
    B2INFO("Phi " << phi0CurvBox.getLowerPhi0Vec());
    B2INFO("Curv " << phi0CurvBox.getLowerCurv());
    B2INFO("Tags of the hits");

    for (const RLTagged<const TrackHit*>& rlTaggedTrackHit : taggedHits) {
      B2INFO("    rl = " << rlTaggedTrackHit.getRLInfo() <<
             " dl = " << rlTaggedTrackHit->getDriftLength());
    }
  }

  B2INFO("First execution took " << timeItResult.getSeconds(0) << " seconds ");
  B2INFO("On average execution took " << timeItResult.getAverageSeconds() << " seconds " <<
         "in " << timeItResult.getNExecutions() << " executions.");

}
