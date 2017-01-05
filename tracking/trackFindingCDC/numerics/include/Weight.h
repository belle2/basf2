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

#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// An additive measure of quality
    using Weight = double;

    /// Generic functor to get the weight from an object.
    struct GetWeight {

      /// Constant returned on an invalid get operation
      static constexpr const Weight c_Invalid = NAN;

      /// Returns the weight of an object.
      template<class T>
      Weight operator()(const T& t) const
      {
        const int dispatchTag = 0;
        return impl(t, dispatchTag);
      }

    private:
      /// Returns the weight of an object. Favored option.
      template <class T>
      static auto impl(const T& t,
                       int favouredTag __attribute__((unused)))
      -> decltype(t.getWeight())
      {
        return t.getWeight();
      }

      /// Returns the weight of an object. Disfavoured option.
      template <class T>
      static auto impl(const T& t,
                       long disfavouredTag __attribute__((unused)))
      -> decltype(t->getWeight())
      {
        return &*t == nullptr ? c_Invalid : t->getWeight();
      }
    };

    /// Operator to sort only according to the weight of the object.
    using GreaterWeight = Greater<GetWeight>;
  }
}
