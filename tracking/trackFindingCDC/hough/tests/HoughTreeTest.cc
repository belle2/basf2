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
#include <tracking/trackFindingCDC/hough/LinearBoxPartition.h>

#include <tracking/trackFindingCDC/legendre/CDCLegendreFastHough.h>

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

namespace {
  /** Predicate class to check for the containment of hits in a phi0 curv hough space part.
   *  Note this part this code defines the performance of
   *  the search in the hough plain quite significantly and there is probably room for improvement.
   */
  template<bool refined = false>
  class HitInPhi0CurvBox {

  public:
    /// Check if four values have the same sign.
    static inline bool sameSign(float n1, float n2, float n3, float n4)
    {
      return ((n1 > 0 and n2 > 0 and n3 > 0 and n4 > 0) or
              (n1 < 0 and n2 < 0 and n3 < 0 and n4 < 0));
    }

    /// Check if two values have the same sign.
    static inline bool sameSign(float n1, float n2)
    {
      return ((n1 > 0 and n2 > 0) or (n1 < 0 and n2 < 0));
    }
  public:
    /** Checks if the track hit is contained in a phi0 curv hough space.
     *  Returns 1.0 if it is contained, returns NAN if it is not contained.
     */
    inline Weight operator()(const TrackHit* hit,
                             const Phi0CurvBox* phi0CurvBox)
    {
      // TODO
      // Replace TrackHit with CDCWireHit or even better CDCRLWireHit !
      // Also get rid of the conformal transformation !
      const CDCWireHit* wireHit = hit->getUnderlayingCDCWireHit();
      return operator()(wireHit, phi0CurvBox);
    }

    /** Checks if the track hit is contained in a phi0 curv hough space.
     *  Returns 1.0 if it is contained, returns NAN if it is not contained.
     *  Accepts if either the right passage hypothesis or the left passage hypothesis
     *  is in the box.
     */
    inline Weight operator()(const CDCWireHit* wireHit,
                             const Phi0CurvBox* phi0CurvBox)
    {
      const FloatType driftLength = wireHit->getRefDriftLength();
      const Vector2D& pos2D =  wireHit->getRefPos2D();
      const FloatType r = wireHit->getRefCylindricalR();

      bool rightIn = this->isObservationIn(r, pos2D, driftLength, phi0CurvBox);
      bool leftIn = this->isObservationIn(r, pos2D, -driftLength, phi0CurvBox);
      return (rightIn or leftIn) ? 1 : NAN;
    }

    /** Checks if the track hit is contained in a phi0 curv hough space.
     *  Returns 1.0 if it is contained, returns NAN if it is not contained.
     *  Accepts if either the right passage hypothesis or the left passage hypothesis
     *  is in the box.
     */
    inline Weight operator()(const CDCRLWireHit* rlWireHit,
                             const Phi0CurvBox* phi0CurvBox)
    {
      const FloatType signedDriftLength = rlWireHit-> getSignedRefDriftLength();
      const Vector2D& pos2D =  rlWireHit->getRefPos2D();
      const FloatType r = rlWireHit->getRefCylindricalR();

      bool in = this->isObservationIn(r, pos2D, signedDriftLength, phi0CurvBox);
      return in ? 1 : NAN;
    }


    inline bool isObservationIn(const Vector2D& pos2D,
                                const FloatType signedDriftLength,
                                const Phi0CurvBox* phi0CurvBox)
    {
      const FloatType r = pos2D.norm();
      return isObservationIn(r, pos2D, signedDriftLength, phi0CurvBox);
    }

    inline bool isObservationIn(const FloatType& r,
                                const Vector2D& pos2D,
                                const FloatType signedDriftLength,
                                const Phi0CurvBox* phi0CurvBox)
    {

      const FloatType rSquared = r * r;

      FloatType orthoToPhi0 [2] = {
        pos2D.cross(phi0CurvBox->getLowerBound<0>().getAngleVec()),
        pos2D.cross(phi0CurvBox->getUpperBound<0>().getAngleVec())
      };

      FloatType rSquareTimesHalfCurve[2] = {
        rSquared* (static_cast<float>(phi0CurvBox->getLowerBound<1>()) / 2),
        rSquared* (static_cast<float>(phi0CurvBox->getUpperBound<1>()) / 2)
      };

      float dist[2][2];
      dist[0][0] = rSquareTimesHalfCurve[0] + orthoToPhi0[0] - signedDriftLength;
      dist[0][1] = rSquareTimesHalfCurve[1] + orthoToPhi0[0] - signedDriftLength;
      dist[1][0] = rSquareTimesHalfCurve[0] + orthoToPhi0[1] - signedDriftLength;
      dist[1][1] = rSquareTimesHalfCurve[1] + orthoToPhi0[1] - signedDriftLength;

      // Sinogram intersects at least on of the boundaries.
      if (not sameSign(dist[0][0], dist[0][1], dist[1][0], dist[1][1])) return true;
      if (not refined) return false;

      // Extra check if the maximum / minimum of the sinogram is in the Box.
      // Check if the slope sign changes
      FloatType parallelToPhi0[2] = {
        pos2D.dot(phi0CurvBox->getLowerBound<0>().getAngleVec()),
        pos2D.dot(phi0CurvBox->getUpperBound<0>().getAngleVec())
      };
      if (sameSign(parallelToPhi0[0], parallelToPhi0[1])) return false;

      // This two values could be precomputed if necessary
      const float curveMax = 2 / (r - signedDriftLength);
      const float curveMin = 2 / (-r - signedDriftLength);
      return (phi0CurvBox->isIn<1>(curveMax) or phi0CurvBox->isIn<1>(curveMin));
      // TODO also extended version does not cover all cases...

    }

  };


}


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
    WeightedFastHough<TrackHit, Phi0CurvBox, LinearBoxPartition<phiDivisions, curvDivisions>::ChildrenStructure>;

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
        B2INFO("Curv " << static_cast<FloatType>(candidate.first.getLowerBound<1>()));
      }
    }

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
