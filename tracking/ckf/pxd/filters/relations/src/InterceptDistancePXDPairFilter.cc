/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/relations/InterceptDistancePXDPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/PXDIntercept.h>

#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoCache.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
InterceptDistancePXDPairFilter::operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation)
{
  const CKFToPXDState& fromState = *(relation.first);
  const CKFToPXDState& toState = *(relation.second);

  const CKFToPXDState::stateCache& fromStateCache = fromState.getStateCache();
  const CKFToPXDState::stateCache& toStateCache = toState.getStateCache();

  if (not fromStateCache.isHitState) {
    // We are coming from an SVD / CDC-SVD track, so we can use its position to only look for matching ladders
    const RecoTrack* seedRecoTrack = fromState.getSeed();
    const auto& relatedIntercepts = seedRecoTrack->getRelationsTo<PXDIntercept>(m_param_PXDInterceptsName);

    // Don't accept relation of no intercept was created for a RecoTrack
    if (relatedIntercepts.size() == 0) {
      return NAN;
    }

    for (const auto& intercept : relatedIntercepts) {
      const VxdID& fromStateSensorID(intercept.getSensorID());
      if (fromStateSensorID.getLayerNumber() != toStateCache.geoLayer) {
        continue;
      }
      const PXD::SensorInfo& sensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(fromStateSensorID));
      const auto& interceptGlobalPoint = sensorInfo.pointToGlobal({intercept.getCoorU(), intercept.getCoorV(), 0});

      float phiDiff = interceptGlobalPoint.Phi() - toStateCache.phi;
      while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
      while (phiDiff < -M_PI) phiDiff += 2. * M_PI;
      const float thetaDiff = interceptGlobalPoint.Theta() - toStateCache.theta;
      if (abs(phiDiff) < static_cast<float>(m_param_PhiInterceptToHitCut) and
          abs(thetaDiff) < static_cast<float>(m_param_ThetaInterceptToHitCut)) {
        return 1.0;
      }
    }
    return NAN;
  }


  B2ASSERT("You have filled the wrong states into this!", toStateCache.isHitState);

  // On same layer we already know from LayerPXDRelationFilter, that we only deal with overlaps in r-phi.
  // So it's sufficient here to check for same layer number to accept states in the overlap region.
  if (fromStateCache.geoLayer == toStateCache.geoLayer and
      fromStateCache.sensorID.getSensorNumber() == toStateCache.sensorID.getSensorNumber()) {
    // TODO: Checking for equality of sensor numbers seems not to harm the hit efficiency,
    // but maybe it's safer to allow for a sensor number difference of 1?
    return 1.0;
  }

  float phiDiff = fromStateCache.phi - toStateCache.phi;
  while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
  while (phiDiff < -M_PI) phiDiff += 2. * M_PI;
  const float thetaDiff = fromStateCache.theta - toStateCache.theta;

  if (abs(phiDiff) < static_cast<float>(m_param_PhiHitHitCut) and
      abs(thetaDiff) < static_cast<float>(m_param_ThetaHitHitCut)) {
    return 1.0;
  }

  return NAN;
}

void InterceptDistancePXDPairFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "phiInterceptToHitCut"), m_param_PhiInterceptToHitCut,
                                "Cut in phi for the difference between PXDIntercept from RecoTrack on the same layer and current hit-based state.",
                                m_param_PhiInterceptToHitCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "thetaInterceptToHitCut"), m_param_ThetaInterceptToHitCut,
                                "Cut in theta for the difference between PXDIntercept from RecoTrack on the same layer and current hit-based state.",
                                m_param_ThetaInterceptToHitCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "phiHitHitCut"), m_param_PhiHitHitCut,
                                "Cut in phi between two hit-based states.", m_param_PhiHitHitCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "thetaHitHitCut"), m_param_ThetaHitHitCut,
                                "Cut in theta between two hit-based states.", m_param_ThetaHitHitCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "PXDInterceptsName"), m_param_PXDInterceptsName,
                                "Name of the PXDIntercepts StoreArray.", m_param_PXDInterceptsName);
}
