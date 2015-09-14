/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>
#include <tracking/trackFindingCDC/numerics/numerics.h>

#include <climits>

namespace Belle2 {

  namespace TrackFindingCDC {

    /** @name Orientation types*/
    /**@{*/

    /// Namespace hiding the constants of the enum
    namespace RightLeftOrientation {
      /// Enumeration to represent the distinct possibilities of the right left passage information
      enum ERightLeft : signed short {
        /// Constant for right passage information
        c_Right = 1,
        /// Constant for an not yet determined right left passage information
        c_Unknown = 0,
        /// Constant for left passage
        c_Left = -1,
        /// Constant for an invalid passage information
        c_Invalid = SHRT_MIN,
      };
    }

    /// Importing only the enum but not the constants
    using ERightLeft = RightLeftOrientation::ERightLeft;

    /// Namespace hiding the constants of the enum
    namespace ForwardBackwardOrientation {
      /// Enumeration to represent the distinct possibilities of the right left passage information
      enum EForwardBackward : signed short {
        /// Constant for a situation where something is more forward
        c_Forward = 1,

        /// Constant for an not yet determined forward or backward arrangement
        c_Unknown = 0,

        /// Constant for a situation where something is more backward
        c_Backward = -1,

        /// Constant for an invalid information
        c_Invalid = SHRT_MIN,

      };
    }

    /// Importing only the enum but not the constants
    using EForwardBackward = ForwardBackwardOrientation::EForwardBackward;


    /// Namespace hiding the constants of the enum
    namespace IncDecOrientation {
      /// Enumeration to represent the distinct possibilities of the right left passage information
      enum EIncDec : signed short {
        /// Constant for a variable that increases
        c_Increasing = 1,

        /// Constant for a variable that is constant
        c_Constant = 0,

        /// Constant for a variable that decreases
        c_Decreasing = -1,

        /// Constant for an invalid information
        c_Invalid = SHRT_MIN,
      };
    }

    /// Importing only the enum but not the constants
    using EIncDec = IncDecOrientation::EIncDec;


    /// Namespace hiding the constants of the enum
    namespace RotationOrientation {
      /// Enumeration to represent the distinct possibilities of the right left passage information
      enum ERotation : signed short {
        /// Constant for counter clockwise oriented circle
        c_CounterClockwise = 1,

        /// For the orientation of a line
        c_Unknown = 0,

        /// Constant for clockwise oriented circle
        c_Clockwise = -1,

        /// Constant for an invalid information
        c_Invalid = SHRT_MIN,
      };
    }

    /// Importing only the enum but not the constants
    using ERotation = RotationOrientation::ERotation;


    /// Return the reversed orientation info. Leaves invalid infos the same.
    template<class AEOrientation>
    inline AEOrientation reversedInfo(const AEOrientation& info)
    { return static_cast<AEOrientation>(-info); }

    /// Return the reversed orientation info. Leaves invalid infos the same.
    template<class AEOrientation>
    inline bool isValidInfo(const AEOrientation& info)
    { return std::abs(info) <= 1; }

    /// Combines two orientation informations to their most likely common one
    /** Returns the average of two orientation information like. \n
     *  ( 1, 1 ) -> 1 \n
     *  ( 1, 0 ) -> 1 \n
     *  ( 1, -1 ) -> 0 \n
     *  ( 0, 0 ) -> 0 \n
     *  plus the inverse and permutation cases. */
    template<class AEOrientation>
    inline AEOrientation averageInfo(const AEOrientation& one, const AEOrientation& two)
    {
      return ((isValidInfo(one) and isValidInfo(two)) ?
              static_cast<AEOrientation>(sign(one + two)) :
              AEOrientation::c_Invalid);
    }

    /// Combines three orientation informations to their most likely common one
    /** Returns the average of two orientation information like. \n
     *  ( 1, 1, 1) -> 1 \n
     *  ( 1, 1, 0) -> 1 \n
     *  ( 1, 1, -1) -> 1 \n
     *  ( 1, 0, 0) -> 1 \n
     *  ( 1, 0, -1) -> 0 \n
     *  ( 0, 0, 0 ) -> 0 \n
     *  plus the inverse and permutation cases. */
    template<class AEOrientation>
    inline AEOrientation averageInfo(const AEOrientation& one,
                                     const AEOrientation& two,
                                     const AEOrientation& three)
    {
      return (isValidInfo(one) and isValidInfo(two) and isValidInfo(three) ?
              static_cast<AEOrientation>(sign(one + two + three)) :
              AEOrientation::c_Invalid);
    }
    /**@}*/

  }
} // namespace Belle2
