/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/utils/DriftLengthEstimator.h>

#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void DriftLengthEstimator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "useAlphaInDriftLength"),
                                m_param_useAlphaInDriftLength,
                                "Switch to serve the alpha angle to the drift length translator",
                                m_param_useAlphaInDriftLength);

  moduleParamList->addParameter(prefixed(prefix, "tofMassScale"),
                                m_param_tofMassScale,
                                "Mass to estimate the velocity in the flight time to the hit",
                                m_param_tofMassScale);
}

void DriftLengthEstimator::updateDriftLength(CDCRecoHit2D& recoHit2D)
{
  CDC::RealisticTDCCountTranslator tdcCountTranslator;
  const FlightTimeEstimator& flightTimeEstimator = FlightTimeEstimator::instance();

  Vector2D flightDirection = recoHit2D.getFlightDirection2D();
  Vector2D recoPos2D = recoHit2D.getRecoPos2D();
  double alpha = recoPos2D.angleWith(flightDirection);
  const double beta = 1;
  double flightTimeEstimate = flightTimeEstimator.getFlightTime2D(recoPos2D, alpha, beta);

  const CDCWire& wire = recoHit2D.getWire();
  const CDCHit* hit = recoHit2D.getWireHit().getHit();
  const bool rl = recoHit2D.getRLInfo() == ERightLeft::c_Right;

  if (not m_param_useAlphaInDriftLength) {
    alpha = 0;
  }

  double driftLength =
    tdcCountTranslator.getDriftLength(hit->getTDCCount(),
                                      wire.getWireID(),
                                      flightTimeEstimate,
                                      rl,
                                      wire.getRefZ(),
                                      alpha);

  recoHit2D.setRefDriftLength(driftLength);
  recoHit2D.snapToDriftCircle();
}

void DriftLengthEstimator::updateDriftLength(CDCRecoSegment2D& segment)
{
  for (CDCRecoHit2D& recoHit2D : segment) {
    updateDriftLength(recoHit2D);
  }
}
