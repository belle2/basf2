/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Tag class to facilitate marking of class as a functor in the sense of this code
    struct FunctorTag {
    };

    /// Test whether a given class is a functor
    template <class T>
    constexpr bool isFunctor()
    {
      return std::is_convertible<T, FunctorTag>::value;
    }
  }
}
