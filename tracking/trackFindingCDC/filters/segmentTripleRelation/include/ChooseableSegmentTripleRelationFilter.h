/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentTripleRelation/SegmentTripleRelationFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    // Guard to prevent repeated instantiations
    extern template class ChooseableFilter<SegmentTripleRelationFilterFactory>;
    using ChooseableSegmentTripleRelationFilter = ChooseableFilter<SegmentTripleRelationFilterFactory>;
  }
}
