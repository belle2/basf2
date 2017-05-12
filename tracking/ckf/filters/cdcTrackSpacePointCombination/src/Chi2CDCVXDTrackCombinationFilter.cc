/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcTrackSpacePointCombination/Chi2CDCVXDTrackCombinationFilter.h>
#include <tracking/ckf/findlets/cdcToSpacePoint/SpacePointKalmanUpdateFitter.h>
#include <tracking/ckf/findlets/cdcToSpacePoint/SpacePointAdvanceAlgorithm.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight Chi2CDCVXDTrackCombinationFilter::operator()(const BaseCDCVXDTrackCombinationFilter::Object& pair)
{
  const RecoTrack* recoTrack = pair.first;
  B2ASSERT("RecoTrack should be fitted at this stage!", recoTrack->wasFitSuccessful());

  const std::vector<const SpacePoint*> spacePoints = pair.second;

  genfit::MeasuredStateOnPlane mSoP = recoTrack->getMeasuredStateOnPlaneFromFirstHit();

  double chi2 = 0;
  for (const SpacePoint* spacePoint : spacePoints) {
    if (not m_advanceAlgorithm.extrapolate(mSoP, spacePoint)) {
      return std::nan("");
    }
    chi2 += m_kalmanAlgorithm.kalmanStep(mSoP, spacePoint);
  }

  B2INFO(chi2);

  return -chi2;
}
