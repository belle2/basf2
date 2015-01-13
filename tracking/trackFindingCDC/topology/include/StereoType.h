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

#include <tracking/trackFindingCDC/typedefs/SignType.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Type for the stereo property of the wire
    typedef SignType StereoType;

    /// Constant for an axial wire
    const StereoType AXIAL = 0;

    /// Constant for an stereo wire in the U configuration
    const StereoType STEREO_U = 1;

    /// Constant for an stereo wire in the V configuration
    const StereoType STEREO_V = -1;

    /// Constant for an invalid stereo information
    const StereoType INVALID_STEREOTYPE = -127;

  } // namespace TrackFindingCDC

} // namespace Belle2

#endif // STEREOTYPES
