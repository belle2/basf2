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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/geometry/UncertainParameterLine2D.h>
#include <tracking/trackFindingCDC/geometry/ParameterLine2D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/numerics/Quadratic.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/dataobjects/CDCHit.h>

#include <algorithm>

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

double DriftLengthEstimator::updateDriftLength(CDCRecoHit2D& recoHit2D)
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

  double driftLength = tdcCountTranslator.getDriftLength(hit->getTDCCount(),
                                                         wire.getWireID(),
                                                         flightTimeEstimate,
                                                         rl,
                                                         wire.getRefZ(),
                                                         alpha);
  if (driftLength > -2 and driftLength < 16) {
    bool snapRecoPos = true;
    recoHit2D.setRefDriftLength(driftLength, snapRecoPos);
  }
  return driftLength;
}

void DriftLengthEstimator::updateDriftLength(CDCFacet& facet)
{
  CDC::RealisticTDCCountTranslator tdcCountTranslator;
  const FlightTimeEstimator& flightTimeEstimator = FlightTimeEstimator::instance();

  const UncertainParameterLine2D& line = facet.getFitLine();
  Vector2D flightDirection = line->tangential();
  Vector2D centralPos2D = line->closest(facet.getMiddleWire().getRefPos2D());
  double alpha = centralPos2D.angleWith(flightDirection);
  if (not m_param_useAlphaInDriftLength) {
    alpha = 0;
  }

  auto doUpdate = [&](CDCRLWireHit & rlWireHit, Vector2D recoPos2D) {
    const CDCWire& wire = rlWireHit.getWire();
    const CDCHit* hit = rlWireHit.getWireHit().getHit();
    const bool rl = rlWireHit.getRLInfo() == ERightLeft::c_Right;
    const double beta = 1;
    double flightTimeEstimate = flightTimeEstimator.getFlightTime2D(recoPos2D, alpha, beta);
    double driftLength = tdcCountTranslator.getDriftLength(hit->getTDCCount(),
                                                           wire.getWireID(),
                                                           flightTimeEstimate,
                                                           rl,
                                                           wire.getRefZ(),
                                                           alpha);
    rlWireHit.setRefDriftLength(driftLength);
  };

  doUpdate(facet.getStartRLWireHit(), facet.getStartRecoPos2D());
  doUpdate(facet.getMiddleRLWireHit(), facet.getMiddleRecoPos2D());
  doUpdate(facet.getEndRLWireHit(), facet.getEndRecoPos2D());

  // More accurate implementation
  // double startDriftLength = updateDriftLength(facet.getStartRecoHit2D());
  // facet.getStartRLWireHit().setRefDriftLength(startDriftLength);

  // double middleDriftLength = updateDriftLength(facet.getMiddleRecoHit2D());
  // facet.getMiddleRLWireHit().setRefDriftLength(middleDriftLength);

  // double endDriftLength = updateDriftLength(facet.getEndRecoHit2D());
  // facet.getEndRLWireHit().setRefDriftLength(endDriftLength);
}

void DriftLengthEstimator::updateDriftLength(CDCSegment2D& segment)
{
  for (CDCRecoHit2D& recoHit2D : segment) {
    updateDriftLength(recoHit2D);
  }
}

double DriftLengthEstimator::updateDriftLength(CDCRecoHit3D& recoHit3D,
                                               double tanLambda)
{
  CDC::RealisticTDCCountTranslator tdcCountTranslator;
  const FlightTimeEstimator& flightTimeEstimator = FlightTimeEstimator::instance();

  Vector2D flightDirection = recoHit3D.getFlightDirection2D();
  const Vector3D& recoPos3D = recoHit3D.getRecoPos3D();
  const Vector2D& recoPos2D = recoPos3D.xy();
  double alpha = recoPos2D.angleWith(flightDirection);
  const double beta = 1;
  double flightTimeEstimate = flightTimeEstimator.getFlightTime2D(recoPos2D, alpha, beta);

  if (std::isnan(tanLambda)) {
    tanLambda = recoPos3D.z() / recoPos3D.cylindricalR();
  }
  const double theta = M_PI / 2 - std::atan(tanLambda);
  flightTimeEstimate *= hypot2(1, tanLambda);

  const CDCWire& wire = recoHit3D.getWire();
  const CDCHit* hit = recoHit3D.getWireHit().getHit();
  const bool rl = recoHit3D.getRLInfo() == ERightLeft::c_Right;
  double driftLength =
    tdcCountTranslator.getDriftLength(hit->getTDCCount(),
                                      wire.getWireID(),
                                      flightTimeEstimate,
                                      rl,
                                      recoPos3D.z(),
                                      alpha,
                                      theta,
                                      hit->getADCCount());
  if (driftLength > -2 and driftLength < 16) {
    bool snapRecoPos = true;
    recoHit3D.setRecoDriftLength(driftLength, snapRecoPos);
  }
  return driftLength;
}


void DriftLengthEstimator::updateDriftLength(CDCSegment3D& segment3D,
                                             const double tanLambda)
{
  for (CDCRecoHit3D& recoHit3D : segment3D) {
    updateDriftLength(recoHit3D, tanLambda);
  }
}

void DriftLengthEstimator::updateDriftLength(CDCTrack& track,
                                             const double tanLambda)
{
  for (CDCRecoHit3D& recoHit3D : track) {
    updateDriftLength(recoHit3D, tanLambda);
  }
}
