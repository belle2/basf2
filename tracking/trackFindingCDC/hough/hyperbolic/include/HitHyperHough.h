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
#include <tracking/trackFindingCDC/hough/hyperbolic/HyperHough.h>
#include <tracking/trackFindingCDC/hough/algorithms/FirstOfPairInBox.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/hough/algorithms/HitInHyperBox.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Hough Tree for finding StereoHits with using CDCRecoHit3D as the item type.
     * This implementation uses the HitInHyperBox decision algorithm with 2 divisions for each step.
     * This class is only an alias. The real algorithm can be found in OtherSimpleBoxDivisionHoughTree.
     */
    using HitHyperHough =
      HyperHough<std::pair<CDCRecoHit3D, const CDCRLWireHit*>, FirstOfPairInBox<HitInHyperBox>, 2, 2, 2>;
  }
}
