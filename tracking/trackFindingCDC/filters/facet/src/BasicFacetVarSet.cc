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

  var<named("superlayer_id")>() = superlayerID;

  var<named("start_layer_id")>() = facet.getStartWire().getILayer();
  var<named("start_drift_length")>() = startDriftLength;
  var<named("start_drift_length_sigma")>() = startDriftLengthSigma;

  var<named("middle_layer_id")>() = facet.getMiddleWire().getILayer();
  var<named("middle_drift_length")>() = middleDriftLength;
  var<named("middle_drift_length_sigma")>() = middleDriftLengthSigma;

  var<named("end_layer_id")>() = facet.getEndWire().getILayer();
  var<named("end_drift_length")>() = endDriftLength;
  var<named("end_drift_length_sigma")>() = endDriftLengthSigma;

  return true;
}
