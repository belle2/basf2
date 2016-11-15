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
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/hough/algorithms/RangeInBox.h>
#include <tracking/trackFindingCDC/hough/algorithms/FirstOfPairInBox.h>
#include <tracking/trackFindingCDC/hough/algorithms/SegmentInZ0TanLambdaBox.h>
#include <tracking/trackFindingCDC/hough/algorithms/HitInZ0TanLambdaBox.h>
#include <tracking/trackFindingCDC/hough/algorithms/TwoHitInBoxAlgorithms.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Hough Tree for finding StereoHits with using Segments as the item type.
     * This implementation uses the RangeInBox-Template with the normal HitInZ0TanLambda decision algorithm
     * and typical maximum values for z0 (+- 120) and s-z-slope (+- 75°) with 2 divisions for each step.
     * This class is only an alias. The real algorithm can be found in SimpeBoxDivisionHoughTree.
     */
    using SegmentInZ0TanLambdaAlgorithm =
      FirstOfPairInBox<TwoHitInBoxAlgorithm<SegmentInZ0TanLambdaBox, FirstOfPairInBox<RangeInBox<HitInZ0TanLambdaBox>>>>;
    using SegmentZ0TanLambdaLegendre =
      Z0TanLambdaLegendre<std::pair<std::pair<CDCSegment3D, CDCTrajectorySZ>, const CDCSegment2D*>, SegmentInZ0TanLambdaAlgorithm, 2, 2>;
  }
}
