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

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool FitlessFacetVarSet::extract(const CDCFacet* ptrFacet)
{
  if (not ptrFacet) return false;
  const CDCFacet& facet = *ptrFacet;

  const CDCFacet::Shape shape = facet.getShape();

  const short cellExtend = shape.getCellExtend();
  const short oClockDelta = shape.getOClockDelta();
  const short absOClockDelta = std::abs(oClockDelta);

  const ERightLeft startRLInfo = facet.getStartRLInfo();
  const ERightLeft middleRLInfo = facet.getMiddleRLInfo();
  const ERightLeft endRLInfo = facet.getEndRLInfo();

  const short stableTwist = -sign(shape.getOClockDelta()) * middleRLInfo;
  const bool startToMiddleIsCrossing = startRLInfo != middleRLInfo;
  const bool middleToEndIsCrossing = middleRLInfo != endRLInfo;

  const bool startToMiddleIsLong = shape.getStartToMiddleCellDistance() > shape.getMiddleToEndCellDistance();

  const bool longArmIsCrossing = startToMiddleIsLong ? startToMiddleIsCrossing : middleToEndIsCrossing;
  const bool shortArmIsCrossing = startToMiddleIsLong ? middleToEndIsCrossing : startToMiddleIsCrossing;
  const short iLayerDifference = facet.getStartWireHit().getILayer() - facet.getEndWireHit().getILayer();
  const short absILayerDifference = std::abs(iLayerDifference);

  var<named("superlayer_id")>() = facet.getISuperLayer();
  var<named("cell_extend")>() = cellExtend;
  var<named("oclock_delta")>() = shape.getOClockDelta();
  var<named("abs_oclock_delta")>() = absOClockDelta;
  var<named("layer_id_difference")>() = iLayerDifference;
  var<named("abs_layer_id_difference")>() = absILayerDifference;

  var<named("long_arm_is_crossing")>() = longArmIsCrossing;
  var<named("short_arm_is_crossing")>() = shortArmIsCrossing;
  var<named("stable_twist")>() = stableTwist;

  var<named("crossing_id")>() = std::copysign(100.0 * std::abs(stableTwist) +
                                              10.0 * shortArmIsCrossing +
                                              1.0 * longArmIsCrossing,
                                              stableTwist);

  var<named("shape_id")>() = 100.0 * cellExtend + absOClockDelta;
  return true;
}
