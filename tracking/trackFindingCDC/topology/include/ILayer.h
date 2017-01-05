/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <climits>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// The type of the layer ids enumerating layers within a superlayer.
    using ILayer = signed short ;

    /// Generic functor to get the superlayer id from an object.
    struct GetILayer {

      /// Constant returned on an invalid get operation
      static const ILayer c_Invalid = SHRT_MIN;

      /// Returns the superlayer of an object.
      template<class T>
      ILayer operator()(const T& t) const
      {
        const int dispatchTag = 0;
        return impl(t, dispatchTag);
      }

    private:
      /// Returns the superlayer of an object. Favored option.
      template <class T>
      static auto impl(const T& t,
                       int favouredTag __attribute__((unused)))
      -> decltype(t.getILayer())
      {
        return t.getILayer();
      }

      /// Returns the superlayer of an object. Disfavoured option.
      template <class T>
      static auto impl(const T& t,
                       long disfavouredTag __attribute__((unused)))
      -> decltype(t->getILayer())
      {
        return &*t == nullptr ? c_Invalid : t->getILayer();
      }
    };
  }
}
