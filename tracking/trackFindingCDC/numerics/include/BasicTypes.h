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

#include <cstddef> //for size_t
#include <cstdlib> //for abs
#include <math.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /** @name Basic integer types*/
    /**@{*/

    typedef int Index;
    const Index INVALID_INDEX = -999;

    /// Track id type
    typedef int ITrackType;

    /// Constant to mark an invalid track id
    const ITrackType INVALID_ITRACK = -998;
    /**@}*/

    /** @name Basic floatig point types*/
    /**@{*/
    /// The float type used by the algorithm. Just as necessary for speed.
    typedef double FloatType;
    /**@}*/

  } // namespace TrackFindingCDC

} // namespace Belle2
