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
  double normalizedLambdaDiff = (currentHitData.theta - nextHitData.theta) / (currentHitData.theta + nextHitData.theta);

  // if the connection is possible in u, it should also be possible in v
  // but as there could in principle be a chance between sensors (X.X.1 -> X.(X+-1).2 or X.X.2 -> X.(X+-1).1)
  // check for a similar theta value instead of v
  if (currentHitData.layer == nextHitData.layer) {
    if (fabs(normalizedLambdaDiff) > m_param_SimpleThetaOverlayRegionCut) {
//       B2INFO("same layer c: " << currentHitData.sensorID << "   " << currentHitData.x << "  " << currentHitData.y << "  " << currentHitData.z << "   " << currentHitData.phi << "  " << currentHitData.theta << "    " << normalizedLambdaDiff);
//       B2INFO("same layer n: " <<    nextHitData.sensorID << "   " <<    nextHitData.x << "  " <<    nextHitData.y << "  " <<    nextHitData.z << "   " <<    nextHitData.phi << "  " <<    nextHitData.theta);
      return NAN;
    }
    // The hits are on the same layer but neighbouring ladders and in the overlap region they are in close proximity in phi.
    // And since they passed the condition above, they are also in close proximity in theta.
    // Thus they are close in phi and should be accepted.
    return 1.0;
  }

  double phiDiff = currentHitData.phi - nextHitData.phi;
  while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
  while (phiDiff < -M_PI) phiDiff += 2. * M_PI;

  if (!(fabs(phiDiff) < m_param_SimplePhiCut and fabs(normalizedLambdaDiff) < m_param_SimpleThetaCut)) {
//     B2INFO("c: " << currentHitData.sensorID << "   " << currentHitData.x << "  " << currentHitData.y << "  " << currentHitData.z << "   " << currentHitData.phi << "  " << currentHitData.theta << "    " << normalizedLambdaDiff);
//     B2INFO("n: " <<    nextHitData.sensorID << "   " <<    nextHitData.x << "  " <<    nextHitData.y << "  " <<    nextHitData.z << "   " <<    nextHitData.phi << "  " <<    nextHitData.theta);
    return NAN;
  }

  return 1.0;
}

void SimpleRelationFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "simpleThetaOverlayCut"), m_param_SimpleThetaOverlayRegionCut,
                                "Simple cut in theta for the overlay region of different ladders in the same layer.",
                                m_param_SimpleThetaOverlayRegionCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "simplePhiCut"), m_param_SimplePhiCut,
                                "Simple cut in phi for relations between hits.", m_param_SimplePhiCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "simpleThetaCut"), m_param_SimpleThetaCut,
                                "Simple cut in theta for relations between hits.", m_param_SimpleThetaCut);
}
