/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/utils/DriftLengthEstimator.h>

#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>

#include <tracking/trackingUtilities/eventdata/tracks/CDCTrack.h>

#include <tracking/trackingUtilities/eventdata/segments/CDCSegment3D.h>
#include <tracking/trackingUtilities/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackingUtilities/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCFacet.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>

#include <cdc/topology/CDCWire.h>

#include <tracking/trackingUtilities/geometry/UncertainParameterLine2D.h>
#include <tracking/trackingUtilities/geometry/ParameterLine2D.h>

#include <tracking/trackingUtilities/numerics/ERightLeft.h>
#include <tracking/trackingUtilities/numerics/Quadratic.h>

#include <tracking/trackingUtilities/utilities/StringManipulation.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/dataobjects/CDCHit.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/VectorUtil.h>

#include <Math/Vector3D.h>
#include <Math/Vector2D.h>
#include <Math/VectorUtil.h>

using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;


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

  ROOT::Math::XYVector flightDirection = recoHit2D.getFlightDirection2D();
  ROOT::Math::XYVector recoPos2D = recoHit2D.getRecoPos2D();
  double alpha = ROOT::Math::VectorUtil::DeltaPhi(recoPos2D, flightDirection);
  const double beta = 1;
  double flightTimeEstimate = FlightTimeEstimator::instance().getFlightTime2D(recoPos2D, alpha, beta);

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

  const UncertainParameterLine2D& line = facet.getFitLine();
  ROOT::Math::XYVector flightDirection = line->tangential();
  ROOT::Math::XYVector centralPos2D = line->closest(facet.getMiddleWire().getRefPos2D());
  double alpha = ROOT::Math::VectorUtil::DeltaPhi(centralPos2D, flightDirection);
  if (not m_param_useAlphaInDriftLength) {
    alpha = 0;
  }

  auto doUpdate = [&](CDCRLWireHit & rlWireHit, ROOT::Math::XYVector recoPos2D) {
    const CDCWire& wire = rlWireHit.getWire();
    const CDCHit* hit = rlWireHit.getWireHit().getHit();
    const bool rl = rlWireHit.getRLInfo() == ERightLeft::c_Right;
    const double beta = 1;
    double flightTimeEstimate = FlightTimeEstimator::instance().getFlightTime2D(recoPos2D, alpha, beta);
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

  ROOT::Math::XYVector flightDirection = recoHit3D.getFlightDirection2D();
  const ROOT::Math::XYZVector& recoPos3D = recoHit3D.getRecoPos3D();
  const ROOT::Math::XYVector& recoPos2D = VectorUtil::get2DVector(recoPos3D);
  double alpha = ROOT::Math::VectorUtil::DeltaPhi(recoPos2D, flightDirection);
  const double beta = 1;
  double flightTimeEstimate = FlightTimeEstimator::instance().getFlightTime2D(recoPos2D, alpha, beta);

  if (std::isnan(tanLambda)) {
    tanLambda = recoPos3D.z() / recoPos3D.Rho();
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
