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
  class ModuleParamList;

  namespace TrackFindingCDC {
    /// Filter adapter type that negates the acceptance criterion for cross checks
    template<class AFilter>
    class NegativeFilter : public AFilter {

    private:
      /// Type of the base class
      using Super = AFilter;

    public:
      /// Type of the object to be filtered
      using Object = typename AFilter::Object;

      using Super::Super;

      /// Invert the result
      Weight operator()(const typename AFilter::Object& obj) final;
    };
  }
}
