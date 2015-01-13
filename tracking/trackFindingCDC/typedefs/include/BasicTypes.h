/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef BASICTYPES_H
#define BASICTYPES_H

#include <cstddef> //for size_t
#include <cstdlib> //for abs
#include <math.h>


namespace Belle2 {

  namespace CDCLocalTracking {

    /** @name Basic integer types*/
    /**@{*/

    typedef int Index;
    const Index INVALID_INDEX = -999;

    /// Track id type
    typedef int ITrackType;
    const ITrackType INVALID_ITRACK = -998; ///< Constant to mark an invalid track id

    /**@}*/

    /** @name Basic floatig point types*/
    /**@{*/
    /// The float type used by the algorithm. Just as necessary for speed.
    typedef double FloatType;

    /// An additive measure of quality (e.g. logarithms of probabilities)
    typedef float Weight;

    /// Constant for the highest possible weight
    extern const Weight HIGHEST_WEIGHT;

    /// Constant for the lowest possible weight
    extern const Weight LOWEST_WEIGHT;
    /**@}*/



  } // namespace CDCLocalTracking

} // namespace Belle2
#endif // BASICTYPES
