/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/MVAFilter.dcl.h>

#include <tracking/trackFindingCDC/filters/cluster/BasicClusterVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Background cluster detection based on MVA package.
    class MVABackgroundClusterFilter
      : public MVAFilter<BasicClusterVarSet> {

    public:
      /// Type of the base class.
      using Super = MVAFilter<BasicClusterVarSet>;

    public:
      /// Constructor initialising the MVAFilter with standard training name for this filter.
      MVABackgroundClusterFilter();
    };
  }
}
