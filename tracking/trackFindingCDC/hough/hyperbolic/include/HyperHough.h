/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost, Dmitrii Neverov                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/hough/trees/OtherSimpleBoxDivisionHoughTree.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Hough Tree for finding StereoHits with using CDCRecoHit3D as the item type.
     * This implementation uses the HitInHyperBox decision algorithm with 2 divisions for each step.
     * This class is only an alias. The real algorithm can be found in OtherSimpleBoxDivisionHoughTree.
     */
    template <class AHitPointerType,
              class AHitDecisionAlgorithm,
              size_t alphaDivisions = 2,
              size_t rhoDivisions = 2,
              size_t muDivisions = 2>
    class HyperHough : public OtherSimpleBoxDivisionHoughTree<AHitPointerType,
      AHitDecisionAlgorithm,
      alphaDivisions,
      rhoDivisions,
      muDivisions> {

    private:
      /// Super type
      using Super = OtherSimpleBoxDivisionHoughTree<AHitPointerType,
            AHitDecisionAlgorithm,
            alphaDivisions,
            rhoDivisions,
            muDivisions>;

    public:
      using DecisionAlgorithm = AHitDecisionAlgorithm;

      /// Constructor using the given maximal level setting the maximal values.
      HyperHough()
        : Super(3, 3, 3, 1, 1, 1)
      {
      }
    };
  }
}
