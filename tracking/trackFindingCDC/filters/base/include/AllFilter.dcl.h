/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
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

    /// Filter adapter type that accepts all items (just for reference).
    template<class AFilter>
    class AllFilter : public AFilter {
    public:
      /// Type of the object to be filtered
      using Object = typename AFilter::Object;

    public:
      /// Accept all items.
      Weight operator()(const Object& obj) final;
    };
  }
}
