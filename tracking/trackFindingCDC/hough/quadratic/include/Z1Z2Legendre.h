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
     * Hough Tree for finding StereoHits with using CDCRecoHit3D as the item type.
     * This implementation uses the HitInZ1Z2 decision algorithm with 2 divisions for each step.
     * This class is only an alias. The real algorithm can be found in SimpleBoxDivisionHoughTree.
     */
    template <class AHitPointerType,
              class AHitDecisionAlgorithm,
              size_t z1Divisions = 3,
              size_t z2Divisions = 3>
    class Z1Z2Legendre : public DebugableSimpleBoxDivisionHoughTree<AHitPointerType,
      AHitDecisionAlgorithm,
      z1Divisions,
      z2Divisions> {

    private:
      /// Super type
      using Super = DebugableSimpleBoxDivisionHoughTree<AHitPointerType,
            AHitDecisionAlgorithm,
            z1Divisions,
            z2Divisions>;

    public:
      /// Constructor using the given maximal level setting the maximal values.
      Z1Z2Legendre()
        : Super(2, 2, 1, 1) //FIXME which values are reasonable??
      {
      }

      static const bool m_lookingForQuadraticTracks = true;
    };
  }
}
