/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/relations/CylinderDistancePXDPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoCache.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
CylinderDistancePXDPairFilter::operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation)
{
  const CKFToPXDState& fromState = *(relation.first);
  const CKFToPXDState& toState = *(relation.second);

  const CKFToPXDState::stateCache& fromStateCache = fromState.getStateCache();
  const CKFToPXDState::stateCache& toStateCache = toState.getStateCache();

  B2ASSERT("You have filled the wrong states into this!", toStateCache.isHitState);

  if (not fromStateCache.isHitState) {
    // We are coming from an SVD / CDC-SVD track, so we can use its position to only look for matching ladders
    // RecoTracks were fitted before CKF, so the measured states on planes should exist
    const RecoTrack* seedRecoTrack = fromState.getSeed();
    genfit::MeasuredStateOnPlane measuredStateOnPlane =
      m_param_isBackwardCKF ? seedRecoTrack->getMeasuredStateOnPlaneFromFirstHit() : seedRecoTrack->getMeasuredStateOnPlaneFromLastHit();

    bool extrapolationSuccessful = true;
    try {
      measuredStateOnPlane.extrapolateToCylinder(c_PXDLayerRadii[toStateCache.geoLayer - 1]);
    }  catch (...) {
      B2DEBUG(20, "ToPXDCKF CylinderDistancePXDPairFilter: extrapolation to cylinder of layer " << toStateCache.geoLayer <<
              " with radius of " << c_PXDLayerRadii[toStateCache.geoLayer - 1] << " failed");
      extrapolationSuccessful = false;
    }

    if (extrapolationSuccessful) {
      const ROOT::Math::XYZVector& positionOnCylinder = ROOT::Math::XYZVector(measuredStateOnPlane.getPos());

      float phiDiff = positionOnCylinder.Phi() - toStateCache.phi;
      while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
      while (phiDiff < -M_PI) phiDiff += 2. * M_PI;
      const float thetaDiff = positionOnCylinder.Theta() - toStateCache.theta;
      if (std::abs(phiDiff) < static_cast<float>(m_param_CylinderExtrapolationToHitPhiCut) and
          std::abs(thetaDiff) < static_cast<float>(m_param_CylinderExtrapolationToHitThetaCut)) {
        return 1.0;
      } else {
        return NAN;
      }
    } else {
      // We don't have have an extrapolated position on the layer of the ToState based on this Seed (RecoTrack), so use simple angular filters.
      float phiDiff = fromStateCache.phi - toStateCache.phi;
      while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
      while (phiDiff < -M_PI) phiDiff += 2. * M_PI;
      const float thetaDiff = fromStateCache.theta - toStateCache.theta;
      if (std::abs(phiDiff) < static_cast<float>(m_param_RecoTrackToHitPhiCut) and
          std::abs(thetaDiff) < static_cast<float>(m_param_RecoTrackToHitThetaCut)) {
        return 1.0;
      }
      return NAN;
    }
  }

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

  if (std::abs(phiDiff) < static_cast<float>(m_param_HitHitPhiCut) and
      std::abs(thetaDiff) < static_cast<float>(m_param_HitHitThetaCut)) {
    return 1.0;
  }

  return NAN;
}

void CylinderDistancePXDPairFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "isBackwardCKF"), m_param_isBackwardCKF,
                                "Does this CKF extrapolate the tracks backward or forward?",
                                m_param_isBackwardCKF);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "cylinderExtrapolationToHitPhiCut"),
                                m_param_CylinderExtrapolationToHitPhiCut,
                                "Cut in phi for the difference between extrapolated position of the Seed to a cylinder with the radius of the ToState layer and the ToState.",
                                m_param_CylinderExtrapolationToHitPhiCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "cylinderExtrapolationToHitThetaCut"),
                                m_param_CylinderExtrapolationToHitThetaCut,
                                "Cut in theta for the difference between extrapolated position of the Seed to a cylinder with the radius of the ToState layer and the ToState.",
                                m_param_CylinderExtrapolationToHitThetaCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "recoTrackToHitPhiCut"), m_param_RecoTrackToHitPhiCut,
                                "Cut in phi for the difference between RecoTrack information and current hit-based state.",
                                m_param_RecoTrackToHitPhiCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "recoTrackToHitThetaCut"), m_param_RecoTrackToHitThetaCut,
                                "Cut in theta for the difference between RecoTrack information and current hit-based state.",
                                m_param_RecoTrackToHitThetaCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "hitHitPhiCut"), m_param_HitHitPhiCut,
                                "Cut in phi between two hit-based states.", m_param_HitHitPhiCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "hitHitThetaCut"), m_param_HitHitThetaCut,
                                "Cut in theta between two hit-based states.", m_param_HitHitThetaCut);
}
