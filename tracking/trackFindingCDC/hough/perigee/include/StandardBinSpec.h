/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/perigee/Phi0Rep.h>
#include <tracking/trackFindingCDC/hough/perigee/ImpactRep.h>
#include <tracking/trackFindingCDC/hough/perigee/CurvRep.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    namespace PerigeeBinSpec {

      /// Hough tree depth and divisions
      const int maxLevel = 13;

      const int phi0Divisions = 2;
      const int curvDivisions = 2;
      const int impactDivisions = 2;


      // Phi0
      ////////

      // Fixed implicit phi0 range
      // const double maxPhi0 = M_PI;
      // const double minPhi0 = -M_PI;

      const int discretePhi0Overlap = 4;
      const int discretePhi0Width = 5;
      const int nPhi0Bins = std::pow(phi0Divisions, maxLevel);
      const Phi0BinsSpec phi0BinsSpec(nPhi0Bins,
                                      discretePhi0Overlap,
                                      discretePhi0Width);

      /// Curvature
      /////////////

      // const double maxCurv = 0.13;
      const double maxCurv = 0.75;
      const double minCurv = -0.018;
      const double maxCurvAcceptance = 0.13;

      const int discreteCurvOverlap = 4;
      const int discreteCurvWidth = 5;
      const int nCurvBins = std::pow(curvDivisions, maxLevel);
      const CurvBinsSpec curvBinsSpec(minCurv,
                                      maxCurv,
                                      nCurvBins,
                                      discreteCurvOverlap,
                                      discreteCurvWidth);

      // Impact parameter
      ///////////////////

      const double maxImpact = 100;
      const double minImpact = -100;

      const int discreteImpactOverlap = 1;
      const int discreteImpactWidth = 2;
      const int nImpactBins = std::pow(impactDivisions, maxLevel);
      const ImpactBinsSpec impactBinsSpec(minImpact,
                                          maxImpact,
                                          nImpactBins,
                                          discreteImpactOverlap,
                                          discreteImpactWidth);

      // Additional parameters
      const double curlCurv = 0.018;
    }
  }
}
