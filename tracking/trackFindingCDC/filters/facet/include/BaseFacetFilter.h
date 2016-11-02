/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Base class for all facet filters
    using BaseFacetFilter = Filter<CDCFacet>;

  } //end namespace TrackFindingCDC
} //end namespace Belle2
