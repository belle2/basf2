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

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/geometry/Line2D.h>
#include <Eigen/Dense>

namespace Belle2 {
  namespace TrackFindingCDC {

    class FacetFitter {

    public:
      /// Fits a proper line to facet and returns the chi2.
      static double fit(const CDCFacet& facet);

      /// Fit a line the positions xyl and the weights.
      static Line2D fit(Eigen::Matrix<double, 3, 3> xyl,
                        Eigen::Array<double, 3, 1> w,
                        double& chi2);
    }; // end class
  } // end namespace TrackFindingCDC
} // namespace Belle2
