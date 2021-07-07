/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/svd/filters/relations/LooseSVDPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
LooseSVDPairFilter::operator()(const std::pair<const CKFToSVDState*, const CKFToSVDState*>& relation)
{
  const CKFToSVDState::stateCache& currentStateCache = relation.first->getStateCache();
  const CKFToSVDState::stateCache& nextStateCache = relation.second->getStateCache();

  // if the connection is possible in u, it should also be possible in v
  // but as there could in principle be a chance between sensors (X.X.1 -> X.(X+-1).2 or X.X.2 -> X.(X+-1).1)
  // check for a similar theta value instead of v
  if (currentStateCache.geoLayer == nextStateCache.geoLayer) {
    if (fabs(currentStateCache.theta - nextStateCache.theta) > m_param_ThetaOverlayRegionPrecut) {
      return NAN;
    }
  }

  float phiDiff = currentStateCache.phi - nextStateCache.phi;
  while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
  while (phiDiff < -M_PI) phiDiff += 2. * M_PI;

  if (not currentStateCache.isHitState) {
    if (fabs(phiDiff) > m_param_PhiSeedHitPrecut) {
      return NAN;
    }
  } else if (!(fabs(phiDiff) < m_param_PhiHitHitPrecut and
               fabs(currentStateCache.theta - nextStateCache.theta) < m_param_ThetaHitHitPrecut)) {
    return NAN;
  }

  return 1.0;
}

void LooseSVDPairFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "thetaOverlayRegionCut"), m_param_ThetaOverlayRegionPrecut,
                                "Pre-cut in theta for the overlay region.", m_param_ThetaOverlayRegionPrecut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "phiSeedHitCut"), m_param_PhiSeedHitPrecut,
                                "Pre-cut in phi for relations between seed states and hit states.", m_param_PhiSeedHitPrecut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "phiHitHitCut"), m_param_PhiHitHitPrecut,
                                "Pre-cut in phi for relations between hit states.", m_param_PhiHitHitPrecut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "thetaHitHitCut"), m_param_ThetaHitHitPrecut,
                                "Pre-cut in theta for relations between hit states.", m_param_ThetaHitHitPrecut);
}
