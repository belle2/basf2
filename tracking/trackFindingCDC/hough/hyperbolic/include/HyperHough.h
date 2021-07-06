/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/hough/trees/SimpleBoxDivisionHoughTree3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Hough Tree for finding StereoHits with using CDCRecoHit3D as the item type.
     * This implementation uses the HitInHyperBox decision algorithm with 2 divisions for each step.
     * This class is only an alias. The real algorithm can be found in SimpleBoxDivisionHoughTree3D.
     */
    template <class AHitPointerType,
              class AHitDecisionAlgorithm,
              size_t qDivisions = 2,
              size_t pDivisions = 2,
              size_t muDivisions = 2>
    class HyperHough : public SimpleBoxDivisionHoughTree3D<AHitPointerType,
      AHitDecisionAlgorithm,
      qDivisions,
      pDivisions,
      muDivisions> {

    private:
      /// Super type
      using Super = SimpleBoxDivisionHoughTree3D<AHitPointerType,
            AHitDecisionAlgorithm,
            qDivisions,
            pDivisions,
            muDivisions>;

    public:
      /// typedef of the templated AHitDecisionAlgorithm; used to reach methods defined in the algorithm e.g. debugLine, centerX, centerY etc.
      using DecisionAlgorithm = AHitDecisionAlgorithm;

      /// Constructor using the given maximal level setting the maximal values.
      HyperHough()
        : Super(10.0, 0.99, 6.0, 1, 1, 0)
      {
      }
    };
  }
}
