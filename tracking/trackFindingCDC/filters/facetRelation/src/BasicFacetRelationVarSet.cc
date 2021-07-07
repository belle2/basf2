/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
