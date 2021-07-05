/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/AllFacetRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

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
