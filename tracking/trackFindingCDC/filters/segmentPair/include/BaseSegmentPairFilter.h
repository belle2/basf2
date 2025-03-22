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
  namespace TrackFindingCDC {
    class CDCSegmentPair;

    // Guard to prevent repeated instantiations
    // extern template class TrackingUtilities::Filter<CDCSegmentPair>;

    /// Base class for all axial to stereo segment pairs filters
    using BaseSegmentPairFilter = TrackingUtilities::Filter<CDCSegmentPair>;
  }
}
