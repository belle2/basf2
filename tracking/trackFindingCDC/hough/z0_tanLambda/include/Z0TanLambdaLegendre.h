/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
      /// Constructor using the given maximal level setting the maximal values.
      Z0TanLambdaLegendre()
        : Super(140, std::tan(75.0 * M_PI / 180.0), 1, 1)
      {
      }

      static const bool m_lookingForQuadraticTracks = false;
    };
  }
}
