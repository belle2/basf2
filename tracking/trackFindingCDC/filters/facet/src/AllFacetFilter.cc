/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/AllFacetFilter.h"

#include <cmath>
#include <framework/logging/Logger.h>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;






AllFacetFilter::AllFacetFilter()
{
}





CellState AllFacetFilter::isGoodFacet(const CDCRecoFacet&) const
{
  return 3.0;
}





void AllFacetFilter::clear() const
{
}





void AllFacetFilter::initialize()
{
}





void AllFacetFilter::terminate()
{
}

