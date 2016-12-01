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

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSZObservations;
    class CDCTrajectorySZ;

    /// Utility class implementing robust versions of linear sz trajectory line fits.
    class CDCRobustSZFitter {

    public:
      /**
       *  Fit a linear sz trajectory to the reconstructed stereo segment.
       *  It uses the normal fitting algorithm but does so multiple times:
       *  In every iteration, one hit is excluded from the observation set and the rest is fitted.
       *  In the end, the mean over the fitting parameters is built and returned.
       *
       *  Does not estimate the covariances of the fit parameters.
       *
       *  TODO:
       *    - Use the median.
       *    - Use RANSAC instead of Theil-Sen.
       *    - Think about the parameters better.
       */
      CDCTrajectorySZ fitUsingSimplifiedTheilSen(const CDCSZObservations& szObservations) const;
    };
  }
}
