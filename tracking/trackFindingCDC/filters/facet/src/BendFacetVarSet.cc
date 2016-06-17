/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/BendFacetVarSet.h>
#include <assert.h>

namespace {
  double hypot3(double x, double y, double z)
  {
    return hypot(hypot(x, y), z);
  }
}


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

BendFacetVarSet::BendFacetVarSet()
  : Super()
{
}

bool BendFacetVarSet::extract(const CDCFacet* ptrFacet)
{
  bool extracted = extractNested(ptrFacet);
  if (not extracted or not ptrFacet) return false;
  const CDCFacet& facet = *ptrFacet;

  /// Fit tangents
  facet.adjustFitLine();

  const CDCRLWireHit& startRLWirehit = facet.getStartRLWireHit();
  const double startDriftLengthVar = startRLWirehit.getRefDriftLengthVariance();
  const double startDriftLengthSigma = sqrt(startDriftLengthVar);
  const double startWeight = 1.0 / startDriftLengthVar;

  const CDCRLWireHit& middleRLWirehit = facet.getMiddleRLWireHit();
  const double middleDriftLengthVar = middleRLWirehit.getRefDriftLengthVariance();
  const double middleDriftLengthSigma = sqrt(middleDriftLengthVar);
  const double middleWeight = 1.0 / middleDriftLengthVar;

  const CDCRLWireHit& endRLWirehit = facet.getEndRLWireHit();
  const double endDriftLengthVar = endRLWirehit.getRefDriftLengthVariance();
  const double endDriftLengthSigma = sqrt(endDriftLengthVar);
  const double endWeight = 1.0 / endDriftLengthVar;

  const double sumWeight = startWeight + middleWeight + endWeight;

  const ParameterLine2D& startToMiddleLine = facet.getStartToMiddleLine();
  const ParameterLine2D& startToEndLine = facet.getStartToEndLine();
  const ParameterLine2D& middleToEndLine = facet.getMiddleToEndLine();

  const double startDistance = middleToEndLine.distance(startRLWirehit.getRefPos2D()) - startRLWirehit.getSignedRefDriftLength();
  const double middleDistance = startToEndLine.distance(middleRLWirehit.getRefPos2D()) - middleRLWirehit.getSignedRefDriftLength();
  const double endDistance = startToMiddleLine.distance(endRLWirehit.getRefPos2D()) - endRLWirehit.getSignedRefDriftLength();

  const double startOptimalStep = startDistance * startWeight / sumWeight;
  const double middleOptimalStep = middleDistance * middleWeight / sumWeight;
  const double endOptimalStep = endDistance * endWeight / sumWeight;

  const double startChi2 = startDistance * (startDistance - startOptimalStep) * startWeight;
  const double middleChi2 = middleDistance * (middleDistance - middleOptimalStep) * middleWeight;
  const double endChi2 = endDistance * (endDistance - endOptimalStep) * endWeight;

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

  const double startToMiddleSigmaPhi = startDriftLengthSigma / startToMiddleLength;
  const double startToEndSigmaPhi = startDriftLengthSigma / startToEndLength;

  const double middleToStartSigmaPhi = middleDriftLengthSigma / startToMiddleLength;
  const double middleToEndSigmaPhi = middleDriftLengthSigma / middleToEndLength;

  const double endToStartSigmaPhi = endDriftLengthSigma / startToEndLength;
  const double endToMiddleSigmaPhi = endDriftLengthSigma / middleToEndLength;

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



  var<named("start_phi")>() = startPhi;
  var<named("start_phi_sigma")>() = startPhiSigma;
  var<named("start_phi_pull")>() = startPhiPull;
  var<named("start_d")>() = startDistance;
  var<named("start_chi2")>() = startChi2;

  var<named("middle_phi")>() = middlePhi;
  var<named("middle_phi_sigma")>() = middlePhiSigma;
  var<named("middle_phi_pull")>() = middlePhiPull;
  var<named("middle_d")>() = middleDistance;
  var<named("middle_chi2")>() = middleChi2;

  var<named("end_phi")>() = endPhi;
  var<named("end_phi_sigma")>() = endPhiSigma;
  var<named("end_phi_pull")>() = endPhiPull;
  var<named("end_d")>() = endDistance;
  var<named("end_chi2")>() = endChi2;

  return true;
}
