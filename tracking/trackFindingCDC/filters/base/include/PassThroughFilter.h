/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter adapter type that passes the weight of a weighted relation as a result
    template<class AFilter>
    class PassThroughFilter : public AFilter {
    public:
      /// Accept all items.
      Weight operator()(const typename AFilter::Object& obj) final {
        return obj.getWeight();
      }

    };
  }
}
