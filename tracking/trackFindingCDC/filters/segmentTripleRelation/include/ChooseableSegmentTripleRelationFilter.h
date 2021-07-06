/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentTripleRelation/BaseSegmentTripleRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTripleRelation/SegmentTripleRelationFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    // Guard to prevent repeated instantiations
    extern template class Chooseable<BaseSegmentTripleRelationFilter>;
    extern template class ChooseableFilter<SegmentTripleRelationFilterFactory>;
    using ChooseableSegmentTripleRelationFilter = ChooseableFilter<SegmentTripleRelationFilterFactory>;
  }
}
