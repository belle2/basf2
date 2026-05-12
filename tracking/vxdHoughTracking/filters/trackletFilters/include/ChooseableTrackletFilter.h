/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/filters/trackletFilters/TrackletFilterFactory.h>
#include <tracking/trackingUtilities/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  namespace vxdHoughTracking {
    /// A chooseable filter for picking out the relations between states
    using ChooseableTrackletFilter = TrackingUtilities::ChooseableFilter<TrackletFilterFactory>;
  }
}
