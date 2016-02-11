/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/BasicFacetVarSet.h>
#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

BasicFacetVarSet::BasicFacetVarSet(const std::string& prefix)
  : Super(prefix)
{
}

bool BasicFacetVarSet::extract(const CDCFacet* ptrFacet)
{
  extractNested(ptrFacet);
  if (not ptrFacet) return false;
  const CDCFacet& facet = *ptrFacet;

  ISuperLayer superlayerID = facet.getISuperLayer();
  const ERightLeft startRLInfo = facet.getStartRLInfo();
  const ERightLeft middleRLInfo = facet.getMiddleRLInfo();
  const ERightLeft endRLInfo = facet.getEndRLInfo();

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

  var<named("start_wire_id")>() = facet.getStartWire().getIWire();
  var<named("start_layer_id")>() = facet.getStartWire().getILayer();
  var<named("start_ref_x")>() = facet.getStartWire().getRefPos2D().x();
  var<named("start_ref_y")>() = facet.getStartWire().getRefPos2D().y();

  var<named("start_rlinfo")>() = startRLInfo;
  var<named("start_drift_length")>() = startDriftLength;
  var<named("start_drift_length_sigma")>() = startDriftLengthSigma;

  var<named("middle_wire_id")>() = facet.getMiddleWire().getIWire();
  var<named("middle_layer_id")>() = facet.getMiddleWire().getILayer();
  var<named("middle_ref_x")>() = facet.getMiddleWire().getRefPos2D().x();
  var<named("middle_ref_y")>() = facet.getMiddleWire().getRefPos2D().y();

  var<named("middle_rlinfo")>() = middleRLInfo;
  var<named("middle_drift_length")>() = middleDriftLength;
  var<named("middle_drift_length_sigma")>() = middleDriftLengthSigma;

  var<named("end_wire_id")>() = facet.getEndWire().getIWire();
  var<named("end_layer_id")>() = facet.getEndWire().getILayer();
  var<named("end_ref_x")>() = facet.getEndWire().getRefPos2D().x();
  var<named("end_ref_y")>() = facet.getEndWire().getRefPos2D().y();

  var<named("end_rlinfo")>() = endRLInfo;
  var<named("end_drift_length")>() = endDriftLength;
  var<named("end_drift_length_sigma")>() = endDriftLengthSigma;

  return true;
}
