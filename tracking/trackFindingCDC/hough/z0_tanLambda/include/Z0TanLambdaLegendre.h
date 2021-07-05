/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/hough/trees/DebugableSimpleBoxDivisionHoughTree.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Hough Tree for finding StereoHits using a templated class as the item type.
     * This implementation uses a templated decision algorithm
     * and typical maximum values for z0 (+- 120) and s-z-slope (+- 75°) with (as default) 2 divisions for each step.
     * This class is more or less only alias. The real algorithm can be found in SimpeBoxDivisionHoughTree.
     */
    template <class AHitPointerType,
              class AHitDecisionAlgorithm,
              size_t z0Divisions = 2,
              size_t tanLambdaDivisions = 2>
    class Z0TanLambdaLegendre : public DebugableSimpleBoxDivisionHoughTree<AHitPointerType,
      AHitDecisionAlgorithm,
      z0Divisions,
      tanLambdaDivisions> {

    private:
      /// Super type
      using Super = DebugableSimpleBoxDivisionHoughTree<AHitPointerType,
            AHitDecisionAlgorithm,
            z0Divisions,
            tanLambdaDivisions>;

    public:
      /// typedef of the templated AHitDecisionAlgorithm; used to reach methods defined in the algorithm e.g. debugLine, centerX, centerY etc.
      using DecisionAlgorithm = AHitDecisionAlgorithm;

      /// Constructor using the given maximal level setting the maximal values.
      Z0TanLambdaLegendre()
        : Super(140, std::tan(75.0 * M_PI / 180.0), 1, 1)
      {
      }
    };
  }
}
