/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

namespace Belle2 {

  namespace TrackingUtilities {
    class CDCTrajectorySZ;
  }
  namespace TrackFindingCDC {
    class CDCSZObservations;

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
      TrackingUtilities::CDCTrajectorySZ fitUsingSimplifiedTheilSen(const CDCSZObservations& szObservations) const;

      /**
       *  Implements the original Theil-Sen line fit algorithm
       *
       *  Does not estimate the covariances of the fit parameters.
       */
      TrackingUtilities::CDCTrajectorySZ fitTheilSen(const CDCSZObservations& szObservations) const;

      /**
       *  Implements the weighted Theil-Sen line fit algorithm
       *
       *  Does not estimate the covariances of the fit parameters.
       */
      TrackingUtilities::CDCTrajectorySZ fitWeightedTheilSen(const CDCSZObservations& szObservations) const;


    private:
      /// Compute the median z0 intercept from the given observations and an estimated slope.
      double getMedianZ0(const CDCSZObservations& szObservations, double tanLambda) const;
    };
  }
}
