/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth <thomas.hauth@kit.edu>                      *
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
  const size_t phi0Divisions = 2; // Division at each level
  const size_t curvDivisions = 2; // Division at each level

  // const size_t maxLevel = 8;
  // const size_t phiDivisions = 3;
  // const size_t curvDivisions = 3;

  using TrackHitPhi0CurvLegendre = HitPhi0CurvLegendre<TrackHit, phi0Divisions, curvDivisions>;
  TrackHitPhi0CurvLegendre trackHitPhi0CurvLegendre;
  trackHitPhi0CurvLegendre.initialize();

  // Get the hits form the test event
  markAllHitsAsUnused();
  std::set<TrackHit*>& hits_set = getHitSet();
  std::vector<TrackHit*> hitVector;

  for (TrackHit* trackHit : hits_set) {
    if (trackHit->getSuperlayerId() % 2 == 0)
      hitVector.push_back(trackHit);
  }

  // Execute the finding a couple of time to find a stable execution time.
  vector< pair<Phi0CurvBox, vector<TrackHit*> > > candidates;

  // Is this still C++? Looks like JavaScript to me.
  TimeItResult timeItResult = timeIt(100, true, [&]() {
    trackHitPhi0CurvLegendre.seed(hitVector);

    const double minWeight = 30.0;
    const double maxCurv = 0.05;
    candidates = trackHitPhi0CurvLegendre.find(minWeight, maxCurv);

    // B2INFO("Execution " << iExecution);
    /// Check if exactly two candidates have been found
    ASSERT_EQ(numberOfPossibleTrackCandidate, candidates.size());

    // Check for the parameters of the track candidates
    // The actual hit numbers are more than 30, but this is somewhat a lower bound
    EXPECT_GE(candidates[0].second.size(), 30);
    EXPECT_GE(candidates[1].second.size(), 30);

    // Exclude the timing of the resource release for comparision with the legendre test.
    trackHitPhi0CurvLegendre.fell();
  });

  trackHitPhi0CurvLegendre.raze();

  for (std::pair<Phi0CurvBox, std::vector<TrackHit*> >& candidate : candidates) {
    B2INFO("Candidate");
    B2INFO("size " << candidate.second.size());
    B2INFO("Phi " << candidate.first.getLowerBound<0>().getAngle());
    B2INFO("Curv " << static_cast<float>(candidate.first.getLowerBound<1>()));
  }

  B2INFO("First execution took " << timeItResult.getSeconds(0) << " seconds ");
  B2INFO("On average execution took " << timeItResult.getAverageSeconds() << " seconds " <<
         "in " << timeItResult.getNExecutions() << " executions.");

}
