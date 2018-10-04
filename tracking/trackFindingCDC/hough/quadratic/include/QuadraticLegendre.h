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
      /// Constructor using the given maximal level setting the maximal values.
      QuadraticLegendre()
        : Super(2, 2, 1, 1)
      {
      }

      static const bool m_lookingForQuadraticTracks = true;
    };
  }
}
