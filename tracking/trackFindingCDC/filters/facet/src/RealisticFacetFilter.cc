/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/RealisticFacetFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/numerics/Quadratic.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

RealisticFacetFilter::RealisticFacetFilter()
  : m_param_phiPullCut(11)
{
}

RealisticFacetFilter::RealisticFacetFilter(double phiPullCut)
  : m_param_phiPullCut(phiPullCut)
{
}

void RealisticFacetFilter::exposeParameters(ModuleParamList* moduleParamList,
                                            const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  moduleParamList->addParameter(prefixed(prefix, "phiPullCut"),
                                m_param_phiPullCut,
                                "Acceptable angle pull in the angle of adjacent tangents to the "
                                "drift circles.",
                                m_param_phiPullCut);
}

Weight RealisticFacetFilter::operator()(const CDCFacet& facet)
{
  facet.adjustFitLine();

  const CDCRLWireHit& startRLWirehit = facet.getStartRLWireHit();
  const double startDriftLengthVar = startRLWirehit.getRefDriftLengthVariance();
  const double startDriftLengthStd = sqrt(startDriftLengthVar);

  const CDCRLWireHit& middleRLWirehit = facet.getMiddleRLWireHit();
  const double middleDriftLengthVar = middleRLWirehit.getRefDriftLengthVariance();
  const double middleDriftLengthStd = sqrt(middleDriftLengthVar);

  const CDCRLWireHit& endRLWirehit = facet.getEndRLWireHit();
  const double endDriftLengthVar = endRLWirehit.getRefDriftLengthVariance();
  const double endDriftLengthStd = sqrt(endDriftLengthVar);

  const ParameterLine2D& startToMiddleLine = facet.getStartToMiddleLine();
  const ParameterLine2D& startToEndLine = facet.getStartToEndLine();
  const ParameterLine2D& middleToEndLine = facet.getMiddleToEndLine();

  const Vector2D& startToMiddleTangentialVector = startToMiddleLine.tangential();
  const Vector2D& startToEndTangentialVector = startToEndLine.tangential();
  const Vector2D& middleToEndTangentialVector = middleToEndLine.tangential();

  const double startToMiddleLength = startToMiddleTangentialVector.norm();
  const double startToEndLength = startToEndTangentialVector.norm();
  const double middleToEndLength = middleToEndTangentialVector.norm();

  const double startCos = startToMiddleTangentialVector.cosWith(startToEndTangentialVector);
  const double middleCos = startToMiddleTangentialVector.cosWith(middleToEndTangentialVector);
  const double endCos = startToEndTangentialVector.cosWith(middleToEndTangentialVector);

  const double startPhi = acos(startCos);
  const double middlePhi = acos(middleCos);
  const double endPhi = acos(endCos);

  const double startToMiddleSigmaPhi = startDriftLengthStd / startToMiddleLength;
  const double startToEndSigmaPhi = startDriftLengthStd / startToEndLength;

  const double middleToStartSigmaPhi = middleDriftLengthStd / startToMiddleLength;
  const double middleToEndSigmaPhi = middleDriftLengthStd / middleToEndLength;

  const double endToStartSigmaPhi = endDriftLengthStd / startToEndLength;
  const double endToMiddleSigmaPhi = endDriftLengthStd / middleToEndLength;

  const double startPhiSigma = hypot3(startToEndSigmaPhi - startToMiddleSigmaPhi,
                                      middleToStartSigmaPhi,
                                      endToStartSigmaPhi);

  const double middlePhiSigma = hypot3(startToMiddleSigmaPhi,
                                       middleToStartSigmaPhi + middleToEndSigmaPhi,
                                       endToMiddleSigmaPhi);

  const double endPhiSigma = hypot3(startToEndSigmaPhi,
                                    middleToEndSigmaPhi,
                                    endToStartSigmaPhi - endToMiddleSigmaPhi);

  const double startPhiPull = startPhi / startPhiSigma;
  const double middlePhiPull = middlePhi / middlePhiSigma;
  const double endPhiPull = endPhi / endPhiSigma;

  if (startPhiPull < m_param_phiPullCut and
      middlePhiPull < m_param_phiPullCut and
      endPhiPull < m_param_phiPullCut) {

    // Introducing a mini penalty to distiguish straighter facets as better
    // This is important since otherwise the ordering of the wires takes precedence and biases
    // for counterclockwise tracks.
    double miniPenalty =
      std::fmin(0.1, (startPhiPull + middlePhiPull + endPhiPull) / m_param_phiPullCut / 1000);

    // Good facet contains three points of the track
    // the amount carried by this facet can the adjusted more realistically
    return 3 - miniPenalty;
  } else {
    return NAN;
  }
}
