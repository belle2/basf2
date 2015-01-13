/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef INFOTYPES_H
#define INFOTYPES_H

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/SignType.h>
#include <tracking/cdcLocalTracking/numerics/numerics.h>

namespace Belle2 {

  namespace CDCLocalTracking {

    /** @name Orientation types*/
    /**@{*/
    /// Type for orientation informations
    typedef signed short OrientationInfo;

    const OrientationInfo INVALID_INFO = -32768; ///< Constant for invalid orientation for all InfoTypes
    const OrientationInfo UNKNOWN_INFO = 0; ///<Constant for unknown orientation for all InfoTypes
    const OrientationInfo UNKNOWN = UNKNOWN_INFO; ///< Legacy constant for unknown orientation for all InfoTypes

    typedef OrientationInfo RightLeftInfo ; ///< Indicator for right or left orientation
    const RightLeftInfo RIGHT = 1;  ///< Constant for right orientation
    const RightLeftInfo LEFT = -1;  ///< Constant for left orientation

    typedef OrientationInfo ForwardBackwardInfo; ///< Indicator for forward or backward orientation
    const ForwardBackwardInfo FORWARD = 1; ///< Constant for forward orientation
    const ForwardBackwardInfo BACKWARD = -1; ///< Constant for backward orientation

    typedef OrientationInfo IncDecInfo; ///< Indicator for increasing or decreasing
    const IncDecInfo INCREASING = 1; ///< Constant for increasing quantity
    const IncDecInfo CONSTANT = 1; ///< Constant for increasing quantity
    const IncDecInfo DECREASING = -1; ///< Constant for decreasing quantity

    typedef OrientationInfo CCWInfo; ///< Indicator for clockwise or counterclockwise orientation
    const CCWInfo CCW = 1;    ///< Constant for counterclockwise orientation
    const CCWInfo CW = -1;    ///< Constant for clockwise orientation

    /// Return the reversed orientation info. Leaves invalid infos the same.
    inline OrientationInfo reversed(const OrientationInfo& info)
    { return OrientationInfo(-info); }

    /// Combines two orientation informations to their most likely common one
    /** Returns the average of two orientation information like. \n
     *  ( 1, 1 ) -> 1 \n
     *  ( 1, 0 ) -> 1 \n
     *  ( 1, -1 ) -> 0 \n
     *  ( 0, 0 ) -> 0 \n
     *  plus the inverse and permutation cases. */
    inline OrientationInfo averageInfo(const OrientationInfo& one, const OrientationInfo& two)
    { return sign(one + two); }

    /// Combines three orientation informations to their most likely common one
    /** Returns the average of two orientation information like. \n
     *  ( 1, 1, 1) -> 1 \n
     *  ( 1, 1, 0) -> 1 \n
     *  ( 1, 1, -1) -> 1 \n
     *  ( 1, 0, 0) -> 1 \n
     *  ( 1, 0, -1) -> 0 \n
     *  ( 0, 0, 0 ) -> 0 \n
     *  plus the inverse and permutation cases. */
    inline OrientationInfo averageInfo(
      const OrientationInfo& one,
      const OrientationInfo& two,
      const OrientationInfo& three
    ) { return sign(one + two + three); }
    /**@}*/

  }
} // namespace Belle2
#endif // INFOTYPES_H
