/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Christian Wessel                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/relations/LoosePXDPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
LoosePXDPairFilter::operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation)
{
  const CKFToPXDState::stateCache& currentStateCache = relation.first->getStateCache();
  const CKFToPXDState::stateCache& nextStateCache = relation.second->getStateCache();

  // if the connection is possible in u, it should also be possible in v
  // but as there could in principle be a chance between sensors (X.X.1 -> X.(X+-1).2 or X.X.2 -> X.(X+-1).1)
  // check for a similar theta value instead of v
  if (currentStateCache.geoLayer == nextStateCache.geoLayer) {
    if (fabs(currentStateCache.theta - nextStateCache.theta) > m_param_theta_overlay_region_precut) {
      return NAN;
    }
  }

  if (not currentStateCache.isHitState) {
    if (fabs(currentStateCache.phi - nextStateCache.phi) > m_param_phi_seed_hit_precut) {
      return NAN;
    }
  } else if (!(fabs(currentStateCache.phi - nextStateCache.phi) < m_param_phi_hit_hit_precut and
               fabs(currentStateCache.theta - nextStateCache.theta) < m_param_theta_hit_hit_precut)) {
    return NAN;
  }

  return 1.0;
}

void LoosePXDPairFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "thetaOverlayRegionCut"), m_param_theta_overlay_region_precut,
                                "Pre-cut in theta for the overlay region.", m_param_theta_overlay_region_precut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "phiSeedHitCut"), m_param_phi_seed_hit_precut,
                                "Pre-cut in phi for relations between seed states and hit states.", m_param_phi_seed_hit_precut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "phiHitHitCut"), m_param_phi_hit_hit_precut,
                                "Pre-cut in phi for relations between hit states.", m_param_phi_hit_hit_precut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "thetaHitHitCut"), m_param_theta_hit_hit_precut,
                                "Pre-cut in theta for relations between hit states.", m_param_theta_hit_hit_precut);
}
