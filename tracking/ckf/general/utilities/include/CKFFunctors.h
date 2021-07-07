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
  /// Helper Functor to get the Seed of a given result
  struct SeedGetter {
    /// Make it a functor
    explicit operator TrackFindingCDC::FunctorTag();

    /// Returns the seed of an object
    template<class T>
    auto operator()(const T& t) const
    {
      return t.getSeed();
    }
  };

  /// Helper Functor to get the Number of hits of a given result
  struct NumberOfHitsGetter {
    /// Make it a functor
    explicit operator TrackFindingCDC::FunctorTag();

    /// Returns the number of hits of an object
    template<class T>
    auto operator()(const T& t) const
    {
      return t->getHits().size();
    }
  };

  /// Helper Functor to get the arc length of a given result
  struct GetArcLength {
    /// Make it a functor
    explicit operator TrackFindingCDC::FunctorTag();

    /// Returns the weight of an object.
    template<class T, class SFINAE = decltype(&T::getArcLength)>
    double operator()(const T& t) const
    {
      return t.getArcLength();
    }
  };
}
