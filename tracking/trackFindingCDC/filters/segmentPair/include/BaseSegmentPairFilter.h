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

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentPair;

    // Guard to prevent repeated instantiations
    extern template class Filter<CDCSegmentPair>;

    /// Base class for all axial to stereo segment pairs filters
    using BaseSegmentPairFilter = Filter<CDCSegmentPair>;
  }
}
