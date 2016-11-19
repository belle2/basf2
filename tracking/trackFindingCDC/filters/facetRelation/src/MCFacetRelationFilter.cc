/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/MCFacetRelationFilter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MCFacetRelationFilter::MCFacetRelationFilter(bool allowReverse)
  : Super(allowReverse)
  , m_mcFacetFilter(allowReverse)
{
  this->addProcessingSignalListener(&m_mcFacetFilter);
}

Weight MCFacetRelationFilter::operator()(const CDCFacet& fromFacet,
                                         const CDCFacet& toFacet)
{
  // the last wire of the neighbor should not be the same as the start wire of the facet
  if (fromFacet.getStartWire() == toFacet.getEndWire()) {
    return NAN;
  }

  // Despite of that two facets are neighbors if both are true facets
  // That also implies the correct tof alignment of the hits not common to both facets
  Weight fromFacetWeight = m_mcFacetFilter(fromFacet);
  Weight toFacetWeight = m_mcFacetFilter(toFacet);

  bool mcDecision = (not std::isnan(fromFacetWeight)) and (not std::isnan(toFacetWeight));

  // the weight must be -2 because the overlap of the facets is two points
  // so the amount of two facets is 4 points hence the cellular automat
  // must calculate 3 + (-2) + 3 = 4 as cellstate
  // this can of course be adjusted for a more realistic information measure
  // ( together with the facet creator filter)
  return mcDecision ? -2.0 : NAN;
}
