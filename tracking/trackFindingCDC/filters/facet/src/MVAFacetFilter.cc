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

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MVAFacetFilter::MVAFacetFilter()
  : Super(makeUnique<MVAFacetVarSet>(), "FacetFilter", 2.82)
{
}

Weight MVAFacetFilter::predict(const CDCFacet& facet)
{
  return 3 - 0.2 * (1 - Super::predict(facet));
}
