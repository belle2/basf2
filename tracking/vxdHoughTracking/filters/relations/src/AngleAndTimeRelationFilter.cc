/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/filters/relations/AngleAndTimeRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

void AngleAndTimeRelationFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "AngleAndTimeThetaCutDeltaL0"), m_ThetaCutDeltaL0,
                                "Simple cut in theta for the overlay region of different ladders in the same layer.",
                                m_ThetaCutDeltaL0);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "AngleAndTimeThetaCutDeltaL1"), m_ThetaCutDeltaL1,
                                "Simple cut in theta for relations between hits with Delta_Layer = +-1.", m_ThetaCutDeltaL1);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "AngleAndTimeThetaCutDeltaL2"), m_ThetaCutDeltaL2,
                                "Simple cut in theta for relations between hits with Delta_Layer = +-2.", m_ThetaCutDeltaL2);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "AngleAndTimeDeltaUTime"), m_DeltaTU,
                                "Cut on the difference in u-side cluster time between two hits during relation creation.", m_DeltaTU);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "AngleAndTimeDeltaVTime"), m_DeltaTV,
                                "Cut on the difference in v-side cluster time between two hits during relation creation.", m_DeltaTV);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useDeltaTCuts"), m_useDeltaTCuts,
                                "Use the cut on the time difference between two hits during relation creation?", m_useDeltaTCuts);
}

TrackFindingCDC::Weight
AngleAndTimeRelationFilter::operator()(const std::pair<const VXDHoughState*, const VXDHoughState*>& relation)
{
  const VXDHoughState::DataCache& currentHitData = relation.first->getDataCache();
  const VXDHoughState::DataCache& nextHitData = relation.second->getDataCache();

  const double absThetaDiff = abs(currentHitData.theta - nextHitData.theta);

  // if the connection is possible in u, it should also be possible in v, but as there could in principle be a chance that the hits
  // are on different sensors (X.X.1 -> X.(X+-1).2 or X.X.2 -> X.(X+-1).1) check for a similar theta value instead of v
  if (currentHitData.layer == nextHitData.layer) {
    if (absThetaDiff > m_ThetaCutDeltaL0) {
      return NAN;
    }
    // The hits are on the same layer but neighbouring ladders and in the overlap region they are in close proximity in phi.
    // And since they passed the condition above, they are also in close proximity in theta.
    // Thus they are close in phi and should be accepted.
    return 1.0;
  }

  const ushort absLayerDiff = abs(currentHitData.layer - nextHitData.layer);
  if ((absLayerDiff == 1 and absThetaDiff < m_ThetaCutDeltaL1) or
      (absLayerDiff == 2 and absThetaDiff < m_ThetaCutDeltaL2)) {

    if (not m_useDeltaTCuts or
        (m_useDeltaTCuts and
         abs(currentHitData.uTime - nextHitData.uTime) < m_DeltaTU and
         abs(currentHitData.vTime - nextHitData.vTime) < m_DeltaTV)) {
      return 1.0;
    }

  }

  return NAN;

}
