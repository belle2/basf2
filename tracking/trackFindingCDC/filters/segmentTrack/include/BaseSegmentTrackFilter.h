/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/filters/base/Filter.dcl.h>

#include <tracking/trackingUtilities/utilities/WeightedRelation.h>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCTrack;
    class CDCSegment2D;
  }
  namespace TrackFindingCDC {

    // Guard to prevent repeated instantiations
    // extern template class TrackingUtilities::Filter<TrackingUtilities::WeightedRelation<TrackingUtilities::CDCTrack, const TrackingUtilities::CDCSegment2D>>;

    /// Base class for segment to track association filters
    using BaseSegmentTrackFilter =
      TrackingUtilities::Filter<TrackingUtilities::WeightedRelation<TrackingUtilities::CDCTrack, const TrackingUtilities::CDCSegment2D>>;
  }
}
