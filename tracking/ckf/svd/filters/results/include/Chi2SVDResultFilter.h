/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/svd/filters/results/BaseSVDResultFilter.h>
#include <tracking/ckf/svd/entities/CKFToSVDResult.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <tracking/ckf/svd/utilities/SVDAdvancer.h>
#include <tracking/ckf/svd/utilities/SVDKalmanStepper.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /// Base filter for CKF SVD results (on overlap check)
  class Chi2SVDResultFilter : public BaseSVDResultFilter {
  public:
    /// Extract the -chi2 and test for the maximal value
    TrackFindingCDC::Weight operator()(const CKFToSVDResult& result)
    {
      const double chi2 = result.getChi2();

      const RecoTrack* seedTrack = result.getSeed();
      const std::vector<const SpacePoint*> spacePoints = result.getHits();

      // TODO: this is bad, as we actually have already performed this!
      genfit::MeasuredStateOnPlane mSoP = seedTrack->getMeasuredStateOnPlaneFromFirstHit();

      for (const SpacePoint* spacePoint : spacePoints) {
        if (std::isnan(m_advancer.extrapolateToPlane(mSoP, *spacePoint))) {
          return NAN;
        }
        m_kalmanStepper.kalmanStep(mSoP, *spacePoint);
      }

      const genfit::MeasuredStateOnPlane& firstCDCHit = seedTrack->getMeasuredStateOnPlaneFromFirstHit();
      m_advancer.extrapolateToPlane(mSoP, firstCDCHit.getPlane());

      const double distance = (mSoP.getPos() - firstCDCHit.getPos()).Mag();

      if ((chi2 > m_param_maximalChi2) and (distance > m_param_maximalDistanceToCDC)) {
        return NAN;
      }

      return chi2;
    }

    /// Expose the parameter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalChi2"),
                                    m_param_maximalChi2, "Maximal chi2", m_param_maximalChi2);
    }

  private:
    /// Parameter: Maximal chi2
    double m_param_maximalChi2 = 80000;
    /// Parameter: Maximal distance to CDC track
    double m_param_maximalDistanceToCDC = 0.98;

    /// Findlet for advancing
    SVDAdvancer m_advancer;
    /// Findlet for kalman step
    SVDKalmanStepper m_kalmanStepper;
  };
}
