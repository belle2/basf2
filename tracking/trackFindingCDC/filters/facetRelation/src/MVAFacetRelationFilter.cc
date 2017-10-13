/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/MVAFacetRelationFilter.h>


#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::MVA<BaseFacetRelationFilter>;

MVAFacetRelationFilter::MVAFacetRelationFilter()
  : Super(std::make_unique<MVAFacetRelationVarSet>(), "FacetRelationFilter", -2.19)
{
}

Weight MVAFacetRelationFilter::predict(const Relation<const CDCFacet>& facetRelation)
{
  return -2 - 0.2 * (1 - Super::predict(facetRelation));
}
