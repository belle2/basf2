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

#include <tracking/trackFindingCDC/utilities/FunctorTag.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// An additive measure of quality
    using Weight = double;

    /// Generic functor to get the weight from an object.
    struct GetWeight {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Returns the weight of an object.
      template<class T, class SFINAE = decltype(&T::getWeight)>
      Weight operator()(const T& t) const
      {
        return t.getWeight();
      }
    };
  }
}
