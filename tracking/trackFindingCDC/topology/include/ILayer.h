/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
