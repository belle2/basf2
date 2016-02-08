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
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>
#include <tracking/trackFindingCDC/hough/RangeInBox.h>
#include <tracking/trackFindingCDC/hough/FirstOfPairInBox.h>
#include <tracking/trackFindingCDC/hough/z0_tanLambda/HitInZ0TanLambdaBox.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Hough Tree for finding StereoHits with using Segments as the item type.
     * This implementation uses the RangeInBox-Template with the normal HitInZ0TanLambda decision algorithm
     * and typical maximum values for z0 (+- 120) and s-z-slope (+- 75°) with 2 divisions for each step.
     * This class is only a typedef. The real algorithm can be found in SimpeBoxDivisionHoughTree.
     */
    using SegmentZ0TanLambdaLegendre =
      Z0TanLambdaLegendre<std::pair<CDCRecoSegment3D, const CDCRecoSegment2D*>, FirstOfPairInBox<RangeInBox<HitInZ0TanLambdaBox>>, 2, 2>;
  }
}
