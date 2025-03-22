/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
