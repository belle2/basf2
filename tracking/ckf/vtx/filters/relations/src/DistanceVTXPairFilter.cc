/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/filters/relations/DistanceVTXPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
DistanceVTXPairFilter::operator()(const std::pair<const CKFToVTXState*, const CKFToVTXState*>& relation)
{
  const CKFToVTXState& fromState = *(relation.first);
  const CKFToVTXState& toState = *(relation.second);

  const CKFToVTXState::stateCache& fromStateCache = fromState.getStateCache();
  const CKFToVTXState::stateCache& toStateCache = toState.getStateCache();

  B2ASSERT("You have filled the wrong states into this!", toStateCache.isHitState);
  double phiDiff = fromStateCache.phi - toStateCache.phi;
  while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
  while (phiDiff < -M_PI) phiDiff += 2. * M_PI;

  double thetaDiff = fromStateCache.theta - toStateCache.theta;
  if (thetaDiff < 0) thetaDiff = -thetaDiff;
  if (thetaDiff > M_PI) thetaDiff = 2. * M_PI - thetaDiff;

  if (not fromStateCache.isHitState) {
    // We are coming from an CDC track, so we can use its position to only look for matching ladders
    if (abs(phiDiff) < m_param_PhiSeedHitCut and abs(thetaDiff) < m_param_ThetaSeedHitCut) {
      return 1.0;
    }

    return NAN;
  }

  if (fromStateCache.geoLayer == toStateCache.geoLayer and
      fromStateCache.sensorID.getSensorNumber() == toStateCache.sensorID.getSensorNumber()) {
    // TODO: Checking for equality of sensor numbers seems not to harm the hit efficiency,
    // but maybe it's safer to allow for a sensor number difference of 1?
    return 1.0;
  }

  if (abs(phiDiff) < m_param_PhiHitHitCut and abs(thetaDiff) < m_param_ThetaHitHitCut) {
    return 1.0;
  }

  return NAN;
}

void DistanceVTXPairFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "phiSeedHitCut"), m_param_PhiSeedHitCut,
                                "Cut in phi for relations between seed states and hit states.", m_param_PhiSeedHitCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "thetaOverlayRegionCut"), m_param_ThetaSeedHitCut,
                                "Cut in theta for relations between seed states and hit states.", m_param_ThetaSeedHitCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "phiHitHitCut"), m_param_PhiHitHitCut,
                                "Cut in phi for relations between hit states.", m_param_PhiHitHitCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "thetaHitHitCut"), m_param_ThetaHitHitCut,
                                "Cut in theta for relations between hit states.", m_param_ThetaHitHitCut);
}
