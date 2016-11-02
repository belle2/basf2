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

#include <tracking/trackFindingCDC/topology/EStereoKind.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/GetValueType.h>
#include <climits>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// The type of the layer and superlayer ids
    using ISuperLayer = signed short;

    /**
     *  This is a utility class for the free ISuperLayer type.
     *  It provides the basic methods to operate on the ISuperLayer numbers.
     */
    struct ISuperLayerUtil {

      /// Utility classes should not be instantiated
      ISuperLayerUtil() = delete;

      /// Constant representing the total number of cdc superlayers
      static const ISuperLayer c_N = 9;

      /// Constant marking the subdetectors closer to the IP than the CDC.
      static const ISuperLayer c_InnerVolume = -1;

      /// Constant marking the subdetectors further away from the IP than the CDC.
      static const ISuperLayer c_OuterVolume = 9;

      /// Constant making an invalid superlayer id
      static const ISuperLayer c_Invalid = SHRT_MIN;

      /// Indicates if the given number corresponds to a true cdc superlayer - excludes the logic ids for inner and outer volumn
      static bool isInvalid(ISuperLayer iSuperLayer);

      /// Indicates if the given number corresponds to a true cdc superlayer - excludes the logic ids for inner and outer volumn
      static bool isInCDC(ISuperLayer iSuperLayer);

      /// Indicates if the given number corresponds to a logical superlayer - includes the logic ids for inner and outer volumn
      static bool isLogical(ISuperLayer iSuperLayer);

      /// Indicates if the given number corresponds to the logical superlayer of the column inside the CDC
      static bool isInnerVolume(ISuperLayer iSuperLayer);

      /// Indicates if the given number corresponds to the logical superlayer of the volumn outside the CDC
      static bool isOuterVolume(ISuperLayer iSuperLayer);

      /// Returns if the super layer with the given id is axial
      static bool isAxial(ISuperLayer iSuperLayer);

      /// Returns the stereo kind of the super layer.
      static EStereoKind getStereoKind(ISuperLayer iSuperLayer);

      /**
       *  Returns the super layer that is inside of the given super layer.
       *  Return c_Invalid for the inner volume and for invalid super layers
       */
      static ISuperLayer getNextInwards(ISuperLayer iSuperLayer);

      /**
       *  Returns the super layer that is outside of the given super layer.
       *  Return c_Invalid for the outer volume and for invalid super layers
       */
      static ISuperLayer getNextOutwards(ISuperLayer iSuperLayer);

      /**
       *  Returns the common superlayer of two objects
       *  ISuperLayerUtil::c_Invalid if there is no common super layer.
       */
      template<class T1, class T2>
      static ISuperLayer getCommon(const T1& t1, const T2& t2)
      {
        ISuperLayer iSuperLayer = getFrom(t1);
        return iSuperLayer == getFrom(t2) ? iSuperLayer : c_Invalid;
      }

      /**
       *  Returns the common superlayer of hits in a container.
       *  ISuperLayerUtil::c_Invalid if there is no common super layer or the container is empty.
       */
      template<class AHits>
      static ISuperLayer getCommon(const AHits& hits)
      {
        using Hit = GetValueType<AHits>;
        return common(hits, getFrom<Hit>, c_Invalid);
      }

      /** Returns the superlayer of an object */
      template<class T>
      static ISuperLayer getFrom(const T& t)
      { return getFromImpl(t, 0); }

    private:
      /** Returns the superlayer of an object. Favored option. */
      template<class T>
      static auto getFromImpl(const T& t, int) -> decltype(t.getISuperLayer())
      { return t.getISuperLayer(); }

      /** Returns the superlayer of an object. Unfavored option. */
      template<class T>
      static auto getFromImpl(const T& t, long) -> decltype(t->getISuperLayer())
      { return &*t == nullptr ? c_Invalid : t->getISuperLayer(); }

    };

  }

}
