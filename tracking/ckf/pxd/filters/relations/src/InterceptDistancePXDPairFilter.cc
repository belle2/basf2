/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/relations/InterceptDistancePXDPairFilter.h>
#include <tracking/trackingUtilities/filters/base/Filter.icc.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/PXDIntercept.h>

#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoCache.h>

#include <tracking/trackingUtilities/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

#include <cmath>

using namespace Belle2;
using namespace TrackingUtilities;

TrackingUtilities::Weight
InterceptDistancePXDPairFilter::operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation)
{
  const CKFToPXDState& fromState = *(relation.first);
  const CKFToPXDState& toState = *(relation.second);

  const CKFToPXDState::stateCache& fromStateCache = fromState.getStateCache();
  const CKFToPXDState::stateCache& toStateCache = toState.getStateCache();

  B2ASSERT("You have filled the wrong states into this!", toStateCache.isHitState);

  const unsigned int layerDiff = std::abs(fromStateCache.geoLayer - toStateCache.geoLayer);

  float phiDiff = deltaPhi(fromStateCache.phi, toStateCache.phi);
  float etaDiff = deltaEtaFromTheta(fromStateCache.theta, toStateCache.theta);

  // fromState and toState on the same layer, i.e. hits in ladder overlap region
  if (layerDiff == 0) {
    if (std::abs(phiDiff) < static_cast<float>(m_param_PhiOverlapHitHitCut) and
        std::abs(etaDiff) < static_cast<float>(m_param_EtaOverlapHitHitCut)) {
      return 1.0;
    }
    return NAN;
  }

  // if fromState is not HitState, then it is the last hit on SVD or SVD/CDC track
  if (not fromStateCache.isHitState) {
    const RecoTrack* seedRecoTrack = fromState.getSeed();
    const auto& relatedIntercepts = seedRecoTrack->getRelationsTo<PXDIntercept>(m_param_PXDInterceptsName);
    // pT dependent factor, pre-set cut value should correspond to pT>=1GeV
    const float scaleInvPt =
      (fromStateCache.ptSeed < m_param_PtThresholdTrackToHitCut ? (m_param_PtThresholdTrackToHitCut / fromStateCache.ptSeed) : 1.);
    if (relatedIntercepts.size() > 0) {
      // We have PXDIntercepts for this Seed (RecoTrack), so use the intercept position for filtering
      for (const auto& intercept : relatedIntercepts) {
        const VxdID& fromStateSensorID(intercept.getSensorID());
        if (fromStateSensorID.getLayerNumber() != toStateCache.geoLayer) {
          continue;
        }
        const PXD::SensorInfo& sensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::getInstance().getSensorInfo(
                                              fromStateSensorID));
        const auto& interceptGlobalPoint = sensorInfo.pointToGlobal({intercept.getCoorU(), intercept.getCoorV(), 0});
        phiDiff = deltaPhi(interceptGlobalPoint.Phi(), toStateCache.phi);
        etaDiff = deltaEtaFromTheta(interceptGlobalPoint.Theta(), toStateCache.theta);
        if (std::abs(phiDiff) < static_cast<float>(m_param_PhiInterceptToHitCut)*float(layerDiff)*scaleInvPt and
            std::abs(etaDiff) < static_cast<float>(m_param_EtaInterceptToHitCut)*float(layerDiff)*scaleInvPt) {
          return 1.0;
        }
      }
      // We have PXD for this Seed (RecoTrack), but the toState isn't close to any of them -> discard combination
      return NAN;
    } else {
      // We don't have PXDIntercepts for this Seed (RecoTrack), so use simple angular filters.
      // Get eta/theta separation from track angle
      const float dR = fromStateCache.perp - toStateCache.perp;
      const float dZ = fromStateCache.perp / tan(fromStateCache.theta) - toStateCache.perp / tan(toStateCache.theta);
      const float cosThetaSeedhit = dZ / sqrt(dR * dR + dZ * dZ);
      etaDiff = convertThetaToEta(cosThetaSeedhit) - convertThetaToEta(cos(fromStateCache.thetaSeed));
      if (std::abs(phiDiff) < static_cast<float>(m_param_PhiRecoTrackToHitCut)*float(layerDiff)*scaleInvPt and
          std::abs(etaDiff) < static_cast<float>(m_param_EtaRecoTrackToHitCut)*float(layerDiff)*scaleInvPt) {
        return 1.0;
      }
      return NAN;
    }
  }

  // hit-hit relation from Layer-2 to Layer-1
  if (std::abs(phiDiff) < static_cast<float>(m_param_PhiHitHitCut) and
      std::abs(etaDiff) < static_cast<float>(m_param_EtaHitHitCut)) {
    return 1.0;
  }

  return NAN;

}

void InterceptDistancePXDPairFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "ptThresholdTrackToHitCut"), m_param_PtThresholdTrackToHitCut,
                                "Threshold on pT to apply inverse pT scale on cut value.",
                                m_param_PtThresholdTrackToHitCut);
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "phiInterceptToHitCut"), m_param_PhiInterceptToHitCut,
                                "Cut in phi for the difference between PXDIntercept from RecoTrack on the same layer and current hit-based state.",
                                m_param_PhiInterceptToHitCut);
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "etaInterceptToHitCut"), m_param_EtaInterceptToHitCut,
                                "Cut in eta for the difference between PXDIntercept from RecoTrack on the same layer and current hit-based state.",
                                m_param_EtaInterceptToHitCut);
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "phiRecoTrackToHitCut"), m_param_PhiRecoTrackToHitCut,
                                "Cut in phi for the difference between RecoTrack information and current hit-based state.",
                                m_param_PhiRecoTrackToHitCut);
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "etaRecoTrackToHitCut"), m_param_EtaRecoTrackToHitCut,
                                "Cut in eta for the difference between RecoTrack information and current hit-based state.",
                                m_param_EtaRecoTrackToHitCut);
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "phiHitHitCut"), m_param_PhiHitHitCut,
                                "Cut in phi between two hit-based states.", m_param_PhiHitHitCut);
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "etaHitHitCut"), m_param_EtaHitHitCut,
                                "Cut in eta between two hit-based states.", m_param_EtaHitHitCut);
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "phiOverlapHitHitCut"), m_param_PhiOverlapHitHitCut,
                                "Cut in phi between two hit-based states in ladder overlap.", m_param_PhiOverlapHitHitCut);
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "etaOverlapHitHitCut"), m_param_EtaOverlapHitHitCut,
                                "Cut in eta between two hit-based states in ladder overlap.", m_param_EtaOverlapHitHitCut);
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "PXDInterceptsName"), m_param_PXDInterceptsName,
                                "Name of the PXDIntercepts StoreArray.", m_param_PXDInterceptsName);
}

float InterceptDistancePXDPairFilter::deltaPhi(float phi1, float phi2)
{
  float dphi = phi1 - phi2;
  while (dphi > M_PI) dphi -= 2. * M_PI;
  while (dphi < -M_PI) dphi += 2. * M_PI;
  return dphi;
}

float InterceptDistancePXDPairFilter::deltaEtaFromTheta(float theta1, float theta2)
{
  return (convertThetaToEta(cos(theta1)) - convertThetaToEta(cos(theta2)));
}

float InterceptDistancePXDPairFilter::convertThetaToEta(float cosTheta)
{
  if (std::abs(cosTheta) < 1) return -0.5 * log((1.0 - cosTheta) / (1.0 + cosTheta));
  else {
    B2INFO("AngularDistancePXDPairFilter::cosTheta >=1 : " << cosTheta);
    return 0;
  }
}
