/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
