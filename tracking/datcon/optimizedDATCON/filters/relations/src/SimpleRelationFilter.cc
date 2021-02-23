/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/relations/SimpleRelationFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
SimpleRelationFilter::operator()(const std::pair<const HitData*, const HitData*>& relation)
{
  const HitData::DataCache& currentHitData = relation.first->getDataCache();
  const HitData::DataCache& nextHitData = relation.second->getDataCache();

  // intended: (theta1 - theta2) / mean(theta1, theta2), so theoretically this is
  // (theta1 - theta2) / ((theta1 + theta2) / 2) = 2 * (theta1 - theta2) / (theta1 + theta2)
  // but skip the additional factor 2 and just account for this in the cut value
//   const double normalizedLambdaDiff = abs((currentHitData.theta - nextHitData.theta) / (currentHitData.theta + nextHitData.theta));
  const double normalizedLambdaDiff = abs(currentHitData.theta - nextHitData.theta);

  // if the connection is possible in u, it should also be possible in v, but as there could in principle be a chance that the hits
  // are on different sensors (X.X.1 -> X.(X+-1).2 or X.X.2 -> X.(X+-1).1) check for a similar theta value instead of v
  if (currentHitData.layer == nextHitData.layer) {
    if (normalizedLambdaDiff > m_param_SimpleThetaCutDeltaL0) {
//       B2INFO("same layer c: " << currentHitData.sensorID << "   " << currentHitData.x << "  " << currentHitData.y << "  " << currentHitData.z << "  " << currentHitData.theta << "    " << normalizedLambdaDiff);
//       B2INFO("same layer n: " <<    nextHitData.sensorID << "   " <<    nextHitData.x << "  " <<    nextHitData.y << "  " <<    nextHitData.z << "  " <<    nextHitData.theta);
      return NAN;
    }
    // The hits are on the same layer but neighbouring ladders and in the overlap region they are in close proximity in phi.
    // And since they passed the condition above, they are also in close proximity in theta.
    // Thus they are close in phi and should be accepted.
    return 1.0;
  }

  const ushort absLayerDiff = abs(currentHitData.layer - nextHitData.layer);
  if ((absLayerDiff == 1 and normalizedLambdaDiff < m_param_SimpleThetaCutDeltaL1) or
      (absLayerDiff == 2 and normalizedLambdaDiff < m_param_SimpleThetaCutDeltaL2)) {
//     B2INFO("c: " << currentHitData.sensorID << "   " << currentHitData.x << "  " << currentHitData.y << "  " << currentHitData.z << "   "  << currentHitData.theta << "    " << normalizedLambdaDiff);
//     B2INFO("n: " <<    nextHitData.sensorID << "   " <<    nextHitData.x << "  " <<    nextHitData.y << "  " <<    nextHitData.z << "   "  <<    nextHitData.theta);
    return 1.0;
  }

  return NAN;

//   if (!(fabs(phiDiff) < m_param_SimplePhiCut and fabs(normalizedLambdaDiff) < m_param_SimpleThetaCut)) {
// //     B2INFO("c: " << currentHitData.sensorID << "   " << currentHitData.x << "  " << currentHitData.y << "  " << currentHitData.z << "   "  << currentHitData.theta << "    " << normalizedLambdaDiff);
// //     B2INFO("n: " <<    nextHitData.sensorID << "   " <<    nextHitData.x << "  " <<    nextHitData.y << "  " <<    nextHitData.z << "   "  <<    nextHitData.theta);
//     return NAN;
//   }
//
//   return 1.0;
}

void SimpleRelationFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "simpleThetaCutDeltaL0"), m_param_SimpleThetaCutDeltaL0,
                                "Simple cut in theta for the overlay region of different ladders in the same layer.",
                                m_param_SimpleThetaCutDeltaL0);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "simpleThetaCutDeltaL1"), m_param_SimpleThetaCutDeltaL1,
                                "Simple cut in theta for relations between hits with Delta_Layer = +-1.", m_param_SimpleThetaCutDeltaL1);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "simpleThetaCutDeltaL2"), m_param_SimpleThetaCutDeltaL2,
                                "Simple cut in theta for relations between hits with Delta_Layer = +-2.", m_param_SimpleThetaCutDeltaL2);
}
