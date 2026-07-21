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
    class CDCWireHitCluster;
  }
  namespace TrackFindingCDC {

    // Guard to prevent repeated instantiations
    // extern template class TrackingUtilities::Filter<CDCWireHitCluster>;

    /// Base class for wire hit cluster filters
    using BaseClusterFilter = TrackingUtilities::Filter<TrackingUtilities::CDCWireHitCluster>;
  }
}
