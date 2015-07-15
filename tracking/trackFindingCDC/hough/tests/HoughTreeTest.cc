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

#include <tracking/trackFindingCDC/hough/phi0_curv/HitInPhi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/WeightedFastHough.h>
#include <tracking/trackFindingCDC/hough/LinearDivision.h>

// #include <tracking/trackFindingCDC/legendre/CDCLegendreFastHough.h>

#ifdef HAS_CALLGRIND
#include <valgrind/callgrind.h>
#endif

#include <set>
#include <cmath>
#include <vector>
#include <memory>
#include <chrono>
#include <algorithm>
#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


TEST_F(CDCLegendreTestFixture, phi0CurvHoughTreeOnTrackHits)
{
  // Prepare the hough algorithm
  const size_t maxLevel = 12;
  const size_t phiDivisions = 2;
  const size_t curvDivisions = 2;

  // const size_t maxLevel = 8;
  // const size_t phiDivisions = 3;
  // const size_t curvDivisions = 3;

  const double minWeight = 30.0;

  using HitPhi0CurvFastHough =
    WeightedFastHough<TrackHit, Phi0CurvBox, LinearDivision<Phi0CurvBox, phiDivisions, curvDivisions> >;

  DiscreteAngleArray discreteAngles(std::pow(phiDivisions, maxLevel) + 1);
  std::pair<DiscreteAngle, DiscreteAngle> phi0Range(discreteAngles.front(), discreteAngles.back());

  // Look to higher momenta first, if the range has the lower bound at higher momenta

  // DiscreteFloatArray discreteCurvs(-3.0, 0.0, std::pow(curvDivisions, maxLevel) + 1);
  // std::pair<DiscreteFloat, DiscreteFloat > curvRange(discreteCurvs.front(), discreteCurvs.back());
  std::pair<float, float> curvRange(-3.0, 0.0);

  Phi0CurvBox phi0CurvHoughPlain(phi0Range, curvRange);
  HitPhi0CurvFastHough hitPhi0CurvHough(phi0CurvHoughPlain);

  const bool refined = false;
  HitInPhi0CurvBox<refined> hitInPhi0CurvBox;

  // Get the hits form the test event
  markAllHitsAsUnused();
  std::set<TrackHit*>& hits_set = getHitSet();
  std::vector<TrackHit*> hitVector;

  for (TrackHit* trackHit : hits_set) {
    if (trackHit->getSuperlayerId() % 2 == 0)
      hitVector.push_back(trackHit);
  }

  size_t nExecutions = 100;
  std::vector<std::chrono::duration<double>> timeSpans;
  timeSpans.reserve(nExecutions);

  for (size_t iExecution = 0;  iExecution < nExecutions; ++iExecution) {
    // Feed the hits to the hough plain and execute the search
    auto now = std::chrono::high_resolution_clock::now();

    hitPhi0CurvHough.seed(hitVector);

#ifdef HAS_CALLGRIND
    CALLGRIND_START_INSTRUMENTATION;
#endif

    vector< pair<Phi0CurvBox, vector<TrackHit*> > > candidates
      = hitPhi0CurvHough.findHeavyLeavesDisjoint(hitInPhi0CurvBox, maxLevel, minWeight);

#ifdef HAS_CALLGRIND
    CALLGRIND_STOP_INSTRUMENTATION;
#endif

    auto later = std::chrono::high_resolution_clock::now();
    timeSpans.push_back(std::chrono::duration_cast<std::chrono::duration<double>>(later - now));

    // Exclude the timing of the resource release for comparision with the legendre test.
    hitPhi0CurvHough.fell();

    // Only output the candidates in the first execution
    if (iExecution == 0) {
      B2INFO("HoughTree took " << timeSpans.back().count() << " seconds " <<
             "in first execution, found " << candidates.size() << " candidates");
      for (std::pair<Phi0CurvBox, std::vector<TrackHit*> >& candidate : candidates) {
        B2INFO("Candidate");
        B2INFO("size " << candidate.second.size());
        B2INFO("Phi " << candidate.first.getLowerBound<0>().getAngle());
        B2INFO("Curv " << static_cast<float>(candidate.first.getLowerBound<1>()));
      }
    }

    // B2INFO("Execution " << iExecution);
    /// Check if exactly two candidates have been found
    ASSERT_EQ(numberOfPossibleTrackCandidate, candidates.size());

    // Check for the parameters of the track candidates
    // The actual hit numbers are more than 30, but this is somewhat a lower bound
    EXPECT_GE(candidates[0].second.size(), 30);
    EXPECT_GE(candidates[1].second.size(), 30);
  }

  hitPhi0CurvHough.raze();

  std::chrono::duration<double> sumTimeSpan =
    std::accumulate(timeSpans.begin(), timeSpans.end(), std::chrono::duration<double>());

  std::chrono::duration<double> avgTimeSpan = sumTimeSpan / timeSpans.size();

  B2INFO("On average: HoughTree took " << avgTimeSpan.count() << " seconds " <<
         "in " << nExecutions << " executions.");

}
