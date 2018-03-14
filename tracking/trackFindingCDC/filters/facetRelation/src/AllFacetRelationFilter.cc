/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/AllFacetRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight AllFacetRelationFilter::operator()(const CDCFacet& fromFacet, const CDCFacet& toFacet)
{
  // The last wire of the neighbor should not be the same as the start wire of the facet
  // The  default weight must be -2 because the overlap of the facets is two points
  // so the amount of two facets is 4 points hence the cellular automat
  // must calculate 3 + (-2) + 3 = 4 as cellstate
  // this can of course be adjusted for a more realistic information measure
  // ( together with the facet creator filter)
  return fromFacet.getStartRLWireHit().isOnWire(toFacet.getEndWire()) ? NAN : -2;
}
