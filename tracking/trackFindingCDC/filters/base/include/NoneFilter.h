/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter adapter type that rejects all items (just for reference).
    template<class AFilter>
    class NoneFilter : public AFilter {
    public:
      /// Accept all items.
      Weight operator()(const typename AFilter::Object& obj __attribute__((unused))) final {
        return NAN;
      }

    };
  }
}
