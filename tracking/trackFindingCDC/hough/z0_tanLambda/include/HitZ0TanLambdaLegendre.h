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
#include <tracking/trackFindingCDC/hough/z0_tanLambda/Z0TanLambdaLegendre.h>
#include <tracking/trackFindingCDC/hough/algorithms/FirstOfPairInBox.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/hough/algorithms/HitInZ0TanLambdaBox.h>
#include <tracking/trackFindingCDC/hough/algorithms/HitInZ0TanLambdaBoxUsingZ.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Hough Tree for finding StereoHits with using CDCRecoHit3D as the item type.
     * This implementation uses the normal HitInZ0TanLambda decision algorithm
     * and typical maximum values for z0 (+- 120) and s-z-slope (+- 75°) with 2 divisions for each step.
     * This class is only an alias. The real algorithm can be found in SimpleBoxDivisionHoughTree.
     */
    using HitZ0TanLambdaLegendre =
      Z0TanLambdaLegendre<std::pair<CDCRecoHit3D, const CDCRLWireHit*>, FirstOfPairInBox<HitInZ0TanLambdaBox>, 2, 2>;

    using HitZ0TanLambdaLegendreUsingZ =
      Z0TanLambdaLegendre<std::pair<CDCRecoHit3D, const CDCRLWireHit*>, FirstOfPairInBox<HitInZ0TanLambdaBoxUsingZ>, 2, 2>;
  }
}
