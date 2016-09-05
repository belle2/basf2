/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/SegmentFitter.h>

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBField.h>
#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string SegmentFitter::getDescription()
{
  return "Fits each segment with a selectable method";
}

void SegmentFitter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "karimakiFit"),
                                m_param_karimakiFit,
                                "Switch to select Karimaki method for fitting instead of Riemann fit",
                                m_param_karimakiFit);

  moduleParamList->addParameter(prefixed(prefix, "fitPos"),
                                m_param_fitPosString,
                                "Positional information of the hits to be used in the fit. "
                                "Options are 'recoPos', 'rlDriftCircle', 'wirePos'.",
                                m_param_fitPosString);

  moduleParamList->addParameter(prefixed(prefix, "fitVariance"),
                                m_param_fitVarianceString,
                                "Positional information of the hits to be used in the fit. "
                                "Options are 'unit', 'driftLength', 'pseudo', 'proper'.",
                                m_param_fitVarianceString);

  moduleParamList->addParameter(prefixed(prefix, "updateDriftLength"),
                                m_param_updateDriftLength,
                                "Switch to reestimate the drift length",
                                m_param_updateDriftLength);

  moduleParamList->addParameter(prefixed(prefix, "useAlphaInDriftLength"),
                                m_param_useAlphaInDriftLength,
                                "Switch to serve the alpha angle to the drift length translator",
                                m_param_useAlphaInDriftLength);

  moduleParamList->addParameter(prefixed(prefix, "tofMassScale"),
                                m_param_tofMassScale,
                                "Mass to estimate the velocity in the flight time to the hit",
                                m_param_tofMassScale);
}

void SegmentFitter::initialize()
{
  Super::initialize();
  if (m_param_fitPosString != std::string("")) {
    try {
      m_fitPos = getFitPos(m_param_fitPosString);
    } catch (std::invalid_argument& e) {
      B2ERROR("Unexpected fitPos parameter : '" << m_param_fitPosString);
    }
  }

  if (m_param_fitVarianceString != std::string("")) {
    try {
      m_fitVariance = getFitVariance(m_param_fitVarianceString);
    } catch (std::invalid_argument& e) {
      B2ERROR("Unexpected fitVariance parameter : '" << m_param_fitVarianceString);
    }
  }

  if (m_param_karimakiFit) {
    if (m_fitPos != EFitPos::c_RecoPos) {
      B2WARNING("Karimaki fitter only works with the reconstructed position as input.");
    }
    m_fitPos = EFitPos::c_RecoPos;
  }
}

void SegmentFitter::apply(std::vector<CDCRecoSegment2D>& outputSegments)
{
  // Update the drift length
  if (m_param_updateDriftLength) {
    CDC::RealisticTDCCountTranslator tdcCountTranslator;
    const FlightTimeEstimator& flightTimeEstimator = FlightTimeEstimator::instance();
    for (CDCRecoSegment2D& segment : outputSegments) {
      for (CDCRecoHit2D& recoHit2D : segment) {
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

        if (not std::isnan(m_param_tofMassScale)) {
          double curvature = 2.0 * std::sin(alpha) / recoPos2D.cylindricalR();
          double pt = CDCBFieldUtil::curvatureToAbsMom2D(curvature, recoPos2D);
          flightTimeEstimate = hypot2(1, m_param_tofMassScale / pt);
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
    }
  }

  for (const CDCRecoSegment2D& segment : outputSegments) {

    CDCObservations2D observations2D(m_fitPos, m_fitVariance);
    observations2D.appendRange(segment);
    if (observations2D.size() < 4) {
      segment.getTrajectory2D().clear();
    } else {
      if (m_param_karimakiFit) {
        CDCTrajectory2D trajectory2D = m_karimakiFitter.fit(observations2D);
        segment.setTrajectory2D(trajectory2D);
      } else {
        CDCTrajectory2D trajectory2D = m_riemannFitter.fit(observations2D);
        segment.setTrajectory2D(trajectory2D);
      }
    }
  }
}
