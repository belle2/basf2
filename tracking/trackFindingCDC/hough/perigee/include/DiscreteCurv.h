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

#include <tracking/trackFindingCDC/hough/DiscreteValues.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type for the container of the discrete values
    using DiscreteCurvArray = DiscreteValueArray<float>;

    /// Type for discrete float values
    class DiscreteCurv : public DiscreteValue<float> {

    public:
      /// Exposing the base class constructors
      using DiscreteValue<float>::DiscreteValue;

    public:
      /// Extract the range from an array providing the discrete values.
      static std::pair<DiscreteCurv, DiscreteCurv> getRange(const DiscreteCurvArray& valueArray)
      { return {DiscreteCurv(valueArray.front()), DiscreteCurv(valueArray.back())}; }
    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
