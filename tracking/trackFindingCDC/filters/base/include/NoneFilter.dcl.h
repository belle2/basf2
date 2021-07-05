/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
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

    /// Filter adapter type that rejects all items.
    template<class AFilter>
    class NoneFilter : public AFilter {
    public:
      /// Type of the object to be filtered
      using Object = typename AFilter::Object;

    public:
      /// Rejects all items.
      Weight operator()(const Object& obj) final;
    };
  }
}
