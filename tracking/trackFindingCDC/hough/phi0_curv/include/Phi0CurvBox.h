/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/Box.h>
#include <tracking/trackFindingCDC/hough/DiscreteAngle.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A rectangular domain for the hough space over phi0 and two dimensional curvature.
    typedef Box<DiscreteAngle, float> Phi0CurvBox;

  } // end namespace TrackFindingCDC
} // end namespace Belle2
