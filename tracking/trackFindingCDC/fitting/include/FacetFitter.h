/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/numerics/Matrix.h>

#include <limits>

namespace Belle2 {
  namespace TrackingUtilities {
    class UncertainParameterLine2D;
    class CDCFacet;
  }
  namespace TrackFindingCDC {

    /// Utility class to fit hit triplet and relations of them
    class FacetFitter {

    public:
      /**
       *  Fits a proper line to the facet and returns the chi2.
       *
       *  The fitted line is committed to the facet only if its chi2 is at most maxChi2.
       *  With the default maxChi2 = infinity the line is always committed; passing a
       *  finite maxChi2 skips the construction of the line and its covariance matrix
       *  for facets that fail the cut anyway.
       */
      static double fit(const TrackingUtilities::CDCFacet& facet,
                        int nSteps = 100,
                        double maxChi2 = std::numeric_limits<double>::infinity());

      /**
       *  Fit a line the positions xyl and the weights.
       *
       *  @param fromFacet First facet from the pair of facets
       *  @param toFacet   Second facet from the pair of facets
       *  @param nSteps Maximal number of steps to be taken in the mimisation
       */
      static TrackingUtilities::UncertainParameterLine2D fit(const TrackingUtilities::CDCFacet& fromFacet,
                                                             const TrackingUtilities::CDCFacet& toFacet,
                                                             int nSteps = 100);

      /**
       *  Calculate only the chi2 of a line fitted to the hits of the two facets.
       *
       *  Returns the same value as fit(fromFacet, toFacet, 0).chi2() but avoids
       *  the construction of the line and its covariance matrix.
       *
       *  @param fromFacet First facet from the pair of facets
       *  @param toFacet   Second facet from the pair of facets
       */
      static double fitChi2(const TrackingUtilities::CDCFacet& fromFacet,
                            const TrackingUtilities::CDCFacet& toFacet);

      /**
       *  Fit a line the positions xyl and the weights.
       *
       *  Fits a line to a number of observations with tunable
       *  acurracy versus execution speed.
       *  Special implementations for nSteps = 0 and nSteps=1 have been integrated.
       *  High nSteps use a general minimization method.
       *
       *  @param xyl    A matrix of drift length observations
       *  @param w      An array of weights corresponding to the observations
       *  @param nSteps Maximal number of steps to be taken in the mimisation
       */
      static TrackingUtilities::UncertainParameterLine2D fit(TrackingUtilities::Matrix<double, 3, 3> xyl,
                                                             TrackingUtilities::Matrix<double, 3, 1> w,
                                                             int nSteps = 100);
    };
  }
}
