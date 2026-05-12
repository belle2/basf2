
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/MVAFacetFilter.h>


#include <tracking/trackingUtilities/filters/base/MVAFilter.icc.h>
#include <tracking/trackingUtilities/filters/base/FilterVarSet.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

template class TrackingUtilities::FilterVarSet<Chi2FacetFilter>;
template class TrackingUtilities::MVA<BaseFacetFilter>;

MVAFacetFilter::MVAFacetFilter()
  : Super(std::make_unique<MVAFacetVarSet>(), "trackfindingcdc_FacetFilter", 2.805, "trackfindingcdc_FacetFilterParameters")
{
}

Weight MVAFacetFilter::predict(const CDCFacet& facet)
{
  return 3 - 0.2 * (1 - Super::predict(facet));
}
