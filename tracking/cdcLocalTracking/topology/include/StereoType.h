/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef STEREOTYPES_H
#define STEREOTYPES_H

#include <cstddef> //for size_t
#include <cstdlib> //for abs
#include <math.h>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

namespace Belle2 {

  namespace CDCLocalTracking {

    /** @name Stereo type */
    /**@{*/
    typedef SignType StereoType; ///< Type for the stereo property of the wire
    const StereoType AXIAL = 0; ///< Constant for an axial wire
    const StereoType STEREO_U = 1; ///< Constant for an stereo wire in the U configuration
    const StereoType STEREO_V = -1;  ///< Constant for an stereo wire in the V configuration

    const StereoType INVALID_STEREOTYPE = -127; ///< Constant for an invalid stereo information
    /**@}*/


  } // namespace CDCLocalTracking

} // namespace Belle2
#endif // BASICTYPES
