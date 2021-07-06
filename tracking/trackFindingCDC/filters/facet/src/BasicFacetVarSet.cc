/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/BasicFacetVarSet.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool BasicFacetVarSet::extract(const CDCFacet* ptrFacet)
{
  if (not ptrFacet) return false;
  const CDCFacet& facet = *ptrFacet;

  const ISuperLayer superlayerID = facet.getISuperLayer();

  const CDCRLWireHit& startRLWirehit = facet.getStartRLWireHit();
  const double startDriftLength = startRLWirehit.getSignedRefDriftLength();
  const double startDriftLengthVar = startRLWirehit.getRefDriftLengthVariance();
  const double startDriftLengthSigma = sqrt(startDriftLengthVar);

  const CDCRLWireHit& middleRLWirehit = facet.getMiddleRLWireHit();
  const double middleDriftLength = middleRLWirehit.getSignedRefDriftLength();
  const double middleDriftLengthVar = middleRLWirehit.getRefDriftLengthVariance();
  const double middleDriftLengthSigma = sqrt(middleDriftLengthVar);

  const CDCRLWireHit& endRLWirehit = facet.getEndRLWireHit();
  const double endDriftLength = endRLWirehit.getSignedRefDriftLength();
  const double endDriftLengthVar = endRLWirehit.getRefDriftLengthVariance();
  const double endDriftLengthSigma = sqrt(endDriftLengthVar);

  const ERightLeft startRLInfo = facet.getStartRLInfo();
  const ERightLeft middleRLInfo = facet.getMiddleRLInfo();
  const ERightLeft endRLInfo = facet.getEndRLInfo();
  const CDCRLWireHitTriple::Shape shape = facet.getShape();
  const short oClockDelta = shape.getOClockDelta();
  const short cellExtend = shape.getCellExtend();
  const short stableTwist = -sign(shape.getOClockDelta()) * middleRLInfo;
  const bool startToMiddleIsCrossing = startRLInfo != middleRLInfo;
  const bool middleToEndIsCrossing = middleRLInfo != endRLInfo;

  var<named("superlayer_id")>() = superlayerID;

  // var<named("start_layer_id")>() = facet.getStartWire().getILayer();
  var<named("start_drift_length")>() = startDriftLength;
  var<named("start_drift_length_sigma")>() = startDriftLengthSigma;

  // var<named("middle_layer_id")>() = facet.getMiddleWire().getILayer();
  var<named("middle_drift_length")>() = middleDriftLength;
  var<named("middle_drift_length_sigma")>() = middleDriftLengthSigma;

  // var<named("end_layer_id")>() = facet.getEndWire().getILayer();
  var<named("end_drift_length")>() = endDriftLength;
  var<named("end_drift_length_sigma")>() = endDriftLengthSigma;

  var<named("oclock_delta")>() = oClockDelta;
  var<named("twist")>() = stableTwist;
  var<named("cell_extend")>() = cellExtend;
  var<named("n_crossing")>() = startToMiddleIsCrossing + middleToEndIsCrossing;

  facet.adjustFitLine();
  UncertainParameterLine2D fitLine = facet.getFitLine();
  double alpha = fitLine->at(0.5).angleWith(fitLine->tangential());
  var<named("alpha")>() = alpha;
  return true;
}
