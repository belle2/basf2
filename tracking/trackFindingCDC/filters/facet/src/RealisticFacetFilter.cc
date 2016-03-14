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

#include <cmath>
#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  double hypot3(double x, double y, double z)
  {
    return hypot(hypot(x, y), z);
  }
}

RealisticFacetFilter::RealisticFacetFilter():
  m_fitlessFacetFilter(true),
  m_param_phiPullCut(11)
{
}

RealisticFacetFilter::RealisticFacetFilter(FloatType phiPullCut):
  m_fitlessFacetFilter(true),
  m_param_phiPullCut(phiPullCut)
{
}

void RealisticFacetFilter::setParameter(const std::string& key, const std::string& value)
{
  if (key == "phi_pull_cut") {
    m_param_phiPullCut = stod(value);
    B2INFO("Filter received parameter '" << key << "' " << value);
  } else {
    m_fitlessFacetFilter.setParameter(key, value);
  }
}

std::map<std::string, std::string> RealisticFacetFilter::getParameterDescription()
{
  std::map<std::string, std::string> des = m_fitlessFacetFilter.getParameterDescription();
  des["phi_pull_cut"] = "Acceptable angle pull in the angle of adjacent tangents to the "
                        "drift circles.";
  return des;
}


CellWeight RealisticFacetFilter::operator()(const CDCFacet& facet)
{
  CellWeight fitlessWeight = m_fitlessFacetFilter(facet);
  if (isNotACell(fitlessWeight)) return NOT_A_CELL;

  facet.adjustLines();

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

  const FloatType startCos = startToMiddleTangentialVector.cosWith(startToEndTangentialVector);
  const FloatType middleCos = startToMiddleTangentialVector.cosWith(middleToEndTangentialVector);
  const FloatType endCos = startToEndTangentialVector.cosWith(middleToEndTangentialVector);

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

  double startPhiPull = startPhi / startPhiSigma;
  double middlePhiPull = middlePhi / middlePhiSigma;
  double endPhiPull = endPhi / endPhiSigma;

  /* cut on the angle of */
  if (startPhiPull < m_param_phiPullCut and
      middlePhiPull <  m_param_phiPullCut and
      endPhiPull < m_param_phiPullCut) {

    // Introducing a mini penilty to distiguish better facets.
    double miniPenalty =
      min(0.1, (startPhiPull + middlePhiPull + endPhiPull) / m_param_phiPullCut / 1000);

    //Good facet contains three points of the track
    // the amount carried by this facet can the adjusted more realistically
    return 3 - miniPenalty;

  } else {

    //B2DEBUG(200,"Rejected facet because flight directions do not match");
    return NOT_A_CELL;

  }

}
