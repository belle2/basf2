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

namespace Belle2 {
  namespace TrackFindingCDC {

    /// The type of the layer ids enumerating layers within a superlayer.
    using ILayer = signed short;

    /// Generic functor to get the superlayer id from an object.
    struct GetILayer {
      /// Returns the superlayer of an object.
      template <class T, class SFINAE = decltype(&T::getILayer)>
      ILayer operator()(const T& t) const
      {
        return t.getILayer();
      }
    };
  }
}
