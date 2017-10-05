/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Matrix.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;
    class UncertainParameterLine2D;

    /// Utility class to fit hit triplet and relations of them
    class FacetFitter {

    public:
      /// Fits a proper line to facet and returns the chi2.
      static double fit(const CDCFacet& facet,
                        int nSteps = 100);

      /**
       *  Fit a line the positions xyl and the weights.
       *
       *  @param fromFacet First facet from the pair of facets
       *  @param toFacet   Second facet from the pair of facets
       *  @param nSteps Maximal number of steps to be taken in the mimisation
       */
      static UncertainParameterLine2D fit(const CDCFacet& fromFacet,
                                          const CDCFacet& toFacet,
                                          int nSteps = 100);

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
      static UncertainParameterLine2D fit(Matrix<double, 3, 3> xyl,
                                          Matrix<double, 3, 1> w,
                                          int nSteps = 100);
    };
  }
}
