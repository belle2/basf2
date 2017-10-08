/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentTriple;

    // Guard to prevent repeated instantiations
    extern template class Filter<CDCSegmentTriple>;

    /// Base filter for the constuction of segment triples.
    using BaseSegmentTripleFilter = Filter<CDCSegmentTriple>;
  }
}
