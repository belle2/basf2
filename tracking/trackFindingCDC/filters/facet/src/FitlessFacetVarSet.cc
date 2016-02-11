/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/FitlessFacetVarSet.h>
#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

FitlessFacetVarSet::FitlessFacetVarSet(const std::string& prefix)
  : Super(prefix)
{
}

bool FitlessFacetVarSet::extract(const CDCFacet* ptrFacet)
{
  extractNested(ptrFacet);
  if (not ptrFacet) return false;
  const CDCFacet& facet = *ptrFacet;

  /// Fitless variables
  CDCFacet::Shape shape = facet.getShape();
  ISuperLayer superlayerID = facet.getISuperLayer();

  const ERightLeft startRLInfo = facet.getStartRLInfo();
  const ERightLeft middleRLInfo = facet.getMiddleRLInfo();
  const ERightLeft endRLInfo = facet.getEndRLInfo();

  short oClockDelta = shape.getOClockDelta();

  short cellExtend = shape.getCellExtend();

  short stableTwist = -sign(shape.getOClockDelta()) * middleRLInfo;
  short absOClockDelta = std::abs(oClockDelta);
  bool startToMiddleIsCrossing = startRLInfo != middleRLInfo;
  bool middleToEndIsCrossing = middleRLInfo != endRLInfo;

  bool startToMiddleIsLong = shape.getStartToMiddleCellDistance() > shape.getMiddleToEndCellDistance();

  bool longArmIsCrossing = startToMiddleIsLong ? startToMiddleIsCrossing : middleToEndIsCrossing;
  bool shortArmIsCrossing = startToMiddleIsLong ? middleToEndIsCrossing : startToMiddleIsCrossing;
  short iLayerDifference = facet.getStartWire().getILayer() - facet.getEndWire().getILayer();
  short absILayerDifference = std::abs(iLayerDifference);

  const CDCRLTaggedWireHit& startRLWirehit = facet.getStartRLWireHit();
  const double startDriftLength = startRLWirehit.getRefDriftLength();
  const double startDriftLengthVar = startRLWirehit.getRefDriftLengthVariance();
  const double startDriftLengthSigma = sqrt(startDriftLengthVar);

  const CDCRLTaggedWireHit& middleRLWirehit = facet.getMiddleRLWireHit();
  const double middleDriftLength = middleRLWirehit.getRefDriftLength();
  const double middleDriftLengthVar = middleRLWirehit.getRefDriftLengthVariance();
  const double middleDriftLengthSigma = sqrt(middleDriftLengthVar);

  const CDCRLTaggedWireHit& endRLWirehit = facet.getEndRLWireHit();
  const double endDriftLength = endRLWirehit.getRefDriftLength();
  const double endDriftLengthVar = endRLWirehit.getRefDriftLengthVariance();
  const double endDriftLengthSigma = sqrt(endDriftLengthVar);

  var<named("superlayer_id")>() = superlayerID;
  var<named("abs_oclock_delta")>() = absOClockDelta;
  var<named("cell_extend")>() = cellExtend;
  var<named("long_arm_is_crossing")>() = longArmIsCrossing;
  var<named("short_arm_is_crossing")>() = shortArmIsCrossing;
  var<named("stable_twist")>() = stableTwist;
  var<named("abs_layer_id_difference")>() = absILayerDifference;

  var<named("crossing_id")>() = std::copysign(100.0 * std::abs(stableTwist) + 10.0 * shortArmIsCrossing + 1.0 * longArmIsCrossing,
                                              stableTwist);
  var<named("shape_id")>() = 100.0 * cellExtend + absOClockDelta;
  var<named("is_forward_progression")>() = (cellExtend + abs(oClockDelta)) <= 6;
  // funny formula, but basically checks the triple to be a progressing forward and not turning in itself.

  var<named("layer_id_difference")>() = iLayerDifference;
  var<named("oclock_delta")>() = shape.getOClockDelta();

  var<named("start_rlinfo")>() = startRLInfo;
  var<named("start_drift_length")>() = startDriftLength;
  var<named("start_drift_length_sigma")>() = startDriftLengthSigma;

  var<named("middle_rlinfo")>() = middleRLInfo;
  var<named("middle_drift_length")>() = middleDriftLength;
  var<named("middle_drift_length_sigma")>() = middleDriftLengthSigma;

  var<named("end_rlinfo")>() = endRLInfo;
  var<named("end_drift_length")>() = endDriftLength;
  var<named("end_drift_length_sigma")>() = endDriftLengthSigma;

  return true;
}
