/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCTrack;

    // Guard to prevent repeated instantiations
    extern template class Filter<WeightedRelation<CDCTrack, const CDCSegment2D>>;

    /// Base class for segment to track association filters
    using BaseSegmentTrackFilter = Filter<WeightedRelation<CDCTrack, const CDCSegment2D>>;
  }
}
