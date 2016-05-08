/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/FitFacetVarSet.h>

#include <tracking/trackFindingCDC/fitting/FacetFitter.h>

#include <assert.h>

using namespace Belle2;
using namespace TrackFindingCDC;

FitFacetVarSet::FitFacetVarSet(const std::string& prefix)
  : Super(prefix)
{
}

bool FitFacetVarSet::extract(const CDCFacet* ptrFacet)
{
  bool extracted = extractNested(ptrFacet);
  if (not extracted or not ptrFacet) return false;
  const CDCFacet& facet = *ptrFacet;

  double chi2 = FacetFitter::fit(facet);

  const ParameterLine2D& fitLine = facet.getStartToEndLine();

  var<named("chi2")>() = chi2;
  var<named("fit_phi")>() = fitLine.tangential().phi();

  const CDCRLWireHit& startRLWireHit = facet.getStartRLWireHit();
  const CDCRLWireHit& middleRLWireHit = facet.getMiddleRLWireHit();
  const CDCRLWireHit& endRLWireHit = facet.getEndRLWireHit();

  const Vector2D startWirePos2D = startRLWireHit.getWire().getRefPos2D();
  const double startL = startRLWireHit.getSignedRefDriftLength();

  const Vector2D middleWirePos2D = middleRLWireHit.getWire().getRefPos2D();
  const double middleL = middleRLWireHit.getSignedRefDriftLength();

  const Vector2D endWirePos2D = endRLWireHit.getWire().getRefPos2D();
  const double endL = endRLWireHit.getSignedRefDriftLength();

  const double startDistance = fitLine.distance(startWirePos2D) - startL;
  const double middleDistance = fitLine.distance(middleWirePos2D) - middleL;
  const double endDistance = fitLine.distance(endWirePos2D) - endL;

  var<named("start_distance")>() = startDistance;
  var<named("middle_distance")>() = middleDistance;
  var<named("end_distance")>() = endDistance;
  var<named("d2")>() = square(startDistance) + square(middleDistance) + square(endDistance);

  return true;
}
