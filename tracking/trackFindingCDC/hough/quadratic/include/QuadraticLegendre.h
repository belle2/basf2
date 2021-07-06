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
     * Hough Tree for finding StereoHits with using CDCRecoHit3D as the item type.
     * This implementation uses the HitInQuadraticBox decision algorithm with 3 divisions for each step.
     * This class is only an alias. The real algorithm can be found in SimpleBoxDivisionHoughTree.
     */
    template <class AHitPointerType,
              class AHitDecisionAlgorithm,
              size_t pDivisions = 3,
              size_t qDivisions = 3>
    class QuadraticLegendre : public DebugableSimpleBoxDivisionHoughTree<AHitPointerType,
      AHitDecisionAlgorithm,
      pDivisions,
      qDivisions> {

    private:
      /// Super type
      using Super = DebugableSimpleBoxDivisionHoughTree<AHitPointerType,
            AHitDecisionAlgorithm,
            pDivisions,
            qDivisions>;

    public:
      /// typedef of the templated AHitDecisionAlgorithm; used to reach methods defined in the algorithm e.g. debugLine, centerX, centerY etc.
      using DecisionAlgorithm = AHitDecisionAlgorithm;

      /// Constructor using the given maximal level setting the maximal values.
      QuadraticLegendre()
        : Super(2, 2, 1, 1)
      {
      }
    };
  }
}
