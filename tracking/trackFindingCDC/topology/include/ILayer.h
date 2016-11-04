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
    using ILayer = signed short ;

    /**
     *  This is a utility class for the free ILayer type.
     *  It provides the basic methods to operate on the ILayer numbers.
     */
    struct ILayerUtil {
      /// Utility classes should not be instantiated.
      ILayerUtil() = delete;

      /// Returns the layer id of an object.
      template<class T>
      static ILayer getFrom(const T& t)
      {
        const int dispatchTag = 0;
        return getFromImpl(t, dispatchTag);
      }

    private:
      /// Returns the layer of an object. Favoured option.
      template <class T>
      static auto getFromImpl(const T& t,
                              int favouredTag __attribute__((unused)))
      -> decltype(t.getILayer())
      {
        return t.getILayer();
      }

      /// Returns the layer of an object. Disfavoured option.
      template <class T>
      static auto getFromImpl(const T& t,
                              long disfavouredTag __attribute__((unused)))
      -> decltype(t->getILayer())
      {
        return t->getILayer();
      }
    };
  }
}
