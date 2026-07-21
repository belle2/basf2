/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/filters/base/Filter.dcl.h>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCSegment2D;
  }
  namespace TrackFindingCDC {

    // Guard to prevent repeated instantiations
    // extern template class TrackingUtilities::Filter<CDCSegment2D>;

    /// Base filter to distinguish fake from real segments.
    using BaseSegmentFilter = TrackingUtilities::Filter<TrackingUtilities::CDCSegment2D>;
  }
}
