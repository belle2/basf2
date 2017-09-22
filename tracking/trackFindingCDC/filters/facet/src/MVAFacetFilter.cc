/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/MVAFacetFilter.h>


#include <tracking/trackFindingCDC/filters/base/MVAFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterVarSet.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::FilterVarSet<Chi2FacetFilter>;
template class TrackFindingCDC::MVA<BaseFacetFilter>;

MVAFacetFilter::MVAFacetFilter()
  : Super(std::make_unique<MVAFacetVarSet>(), "tracking/data/trackfindingcdc_FacetFilter.xml", 2.805)
{
}

Weight MVAFacetFilter::predict(const CDCFacet& facet)
{
  return 3 - 0.2 * (1 - Super::predict(facet));
}
