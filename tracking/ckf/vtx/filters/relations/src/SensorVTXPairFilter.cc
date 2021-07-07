/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/filters/relations/SensorVTXPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

#include <vxd/geometry/GeoCache.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
SensorVTXPairFilter::operator()(const std::pair<const CKFToVTXState*, const CKFToVTXState*>& relation)
{
  const CKFToVTXState& fromState = *(relation.first);
  const CKFToVTXState& toState = *(relation.second);

  const CKFToVTXState::stateCache& fromStateCache = fromState.getStateCache();
  const CKFToVTXState::stateCache& toStateCache = toState.getStateCache();

  B2ASSERT("You have filled the wrong states into this!", toStateCache.isHitState);

  if (not fromStateCache.isHitState) {
    // We are coming from a CDC track, so we can use its position to only look for matching ladders
    // This is done with the sensorCenterPhi, using the state's phi (and/or theta) wouldn't be a SensorFilter anymore.
    double phiDiff = fromStateCache.phi - toStateCache.sensorCenterPhi;
    while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
    while (phiDiff < -M_PI) phiDiff += 2. * M_PI;

    if (fabs(phiDiff) < m_param_PhiRecoTrackToHitCut) {
      return 1.0;
    }
    // If the current state (fromState) is a RecoTrack-based state, but no relations could be created
    // don't proceed but return
    return NAN;
  }

  // On same layer we already know from LayerPXDRelationFilter, that we only deal with overlaps in r-phi.
  // So it's sufficient here to check for same layer number to accept states in the overlap region.
  if (fromStateCache.geoLayer == toStateCache.geoLayer and
      fromStateCache.sensorID.getSensorNumber() == toStateCache.sensorID.getSensorNumber()) {
    return 1.0;
  }

  // Next layer is not an overlap one, so we can just return all hits of the next layer(s)
  // that are close enough in phi. No cut in theta here since this is the SensorVTXPairFilter,
  // a cut in theta is used in the DistanceVTXPairFilter
  const int sensorNumberDifference =
    static_cast<int>(fromStateCache.sensorID.getSensorNumber()) - static_cast<int>(toStateCache.sensorID.getSensorNumber());
  const int layerNumberDifference =
    static_cast<int>(fromStateCache.geoLayer) - static_cast<int>(toStateCache.geoLayer);

  if ((abs(sensorNumberDifference) > 1 and layerNumberDifference == 1) or (abs(sensorNumberDifference) > 2)) {
    return NAN;
  }

  double phiDiff = fromStateCache.sensorCenterPhi - toStateCache.sensorCenterPhi;
  while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
  while (phiDiff < -M_PI) phiDiff += 2. * M_PI;

  if (fabs(phiDiff) < m_param_PhiHitHitCut) {
    return 1.0;
  }

  return NAN;
}

void SensorVTXPairFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "phiRecoTrackToHitCut"), m_param_PhiRecoTrackToHitCut,
                                "Cut in phi for the difference between RecoTrack (seed) mSoP and current hit-based state.", m_param_PhiRecoTrackToHitCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "phiHitHitCut"), m_param_PhiHitHitCut,
                                "Cut in phi between two hit-based states.", m_param_PhiHitHitCut);
}
