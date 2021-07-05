/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
