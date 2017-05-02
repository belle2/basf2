/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/BasicFacetRelationVarSet.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/geometry/ParameterLine2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool BasicFacetRelationVarSet::extract(const Relation<const CDCFacet>* ptrFacetRelation)
{
  if (not ptrFacetRelation) return false;

  const CDCFacet* ptrFromFacet(ptrFacetRelation->getFrom());
  const CDCFacet* ptrToFacet(ptrFacetRelation->getTo());

  const CDCFacet& fromFacet = *ptrFromFacet;
  const CDCFacet& toFacet = *ptrToFacet;

  const ParameterLine2D& fromStartToMiddle = fromFacet.getStartToMiddleLine();
  const ParameterLine2D& fromStartToEnd    = fromFacet.getStartToEndLine();

  const ParameterLine2D& toStartToEnd   = toFacet.getStartToEndLine();
  const ParameterLine2D& toMiddleToEnd  = toFacet.getMiddleToEndLine();

  const double fromMiddleCos = fromStartToMiddle.tangential().cosWith(toStartToEnd.tangential());
  const double toMiddleCos = fromStartToEnd.tangential().cosWith(toMiddleToEnd.tangential());

  const double fromMiddlePhi = acos(fromMiddleCos);
  const double toMiddlePhi = acos(toMiddleCos);

  var<named("superlayer_id")>() = fromFacet.getISuperLayer();
  var<named("from_middle_phi")>() = fromMiddlePhi;
  var<named("to_middle_phi")>() = toMiddlePhi;

  return true;
}
