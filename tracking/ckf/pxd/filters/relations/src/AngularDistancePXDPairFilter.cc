/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/relations/AngularDistancePXDPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
AngularDistancePXDPairFilter::operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation)
{
  const CKFToPXDState& fromState = *(relation.first);
  const CKFToPXDState& toState = *(relation.second);

  const CKFToPXDState::stateCache& fromStateCache = fromState.getStateCache();
  const CKFToPXDState::stateCache& toStateCache = toState.getStateCache();

  B2ASSERT("You have filled the wrong states into this!", toStateCache.isHitState);

  float phiDiff = fromStateCache.phi - toStateCache.phi;
  while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
  while (phiDiff < -M_PI) phiDiff += 2. * M_PI;

  if (not fromStateCache.isHitState) {
    // We are coming from an SVD / CDC-SVD track, so we can use its position to only look for matching ladders
    if (std::abs(phiDiff) < static_cast<float>(m_param_PhiRecoTrackToHitCut)) {
      return 1.0;
    }
    return NAN;
  }

  // On same layer we already know from LayerPXDRelationFilter, that we only deal with overlaps in r-phi.
  // So it's sufficient here to check for same layer number to accept states in the overlap region.
  if (fromStateCache.geoLayer == toStateCache.geoLayer and
      fromStateCache.sensorID.getSensorNumber() == toStateCache.sensorID.getSensorNumber()) {
    // TODO: Checking for equality of sensor numbers seems not to harm the hit efficiency,
    // but maybe it's safer to allow for a sensor number difference of 1?
    return 1.0;
  }

  if (std::abs(phiDiff) < static_cast<float>(m_param_PhiHitHitCut)) {
    return 1.0;
  }

  return NAN;
}

void AngularDistancePXDPairFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "phiRecoTrackToHitCut"), m_param_PhiRecoTrackToHitCut,
                                "Cut in phi for the difference between RecoTrack (seed) mSoP.getPos() and current hit-based state.", m_param_PhiRecoTrackToHitCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "phiHitHitCut"), m_param_PhiHitHitCut,
                                "Cut in phi between two hit-based states.", m_param_PhiHitHitCut);
}
