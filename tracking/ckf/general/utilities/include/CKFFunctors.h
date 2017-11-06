/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#pragma once

#pragma once

namespace Belle2 {
  /// Helper Functor to get the Seed of a given result
  struct SeedGetter {
    /// Make it a functor
    operator TrackFindingCDC::FunctorTag();

    template<class T>
    auto operator()(const T& t) const
    {
      return t.getSeed();
    }
  };

  /// Helper Functor to get the Number of hits of a given result
  struct NumberOfHitsGetter {
    /// Make it a functor
    operator TrackFindingCDC::FunctorTag();

    template<class T>
    auto operator()(const T& t) const
    {
      return t->getHits().size();
    }
  };
}