/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {

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
