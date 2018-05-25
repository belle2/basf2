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
#include <tracking/trackFindingCDC/hough/quadratic/Z1Z2Legendre.h>
#include <tracking/trackFindingCDC/hough/algorithms/FirstOfPairInBox.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/hough/algorithms/HitInZ1Z2Box.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Hough Tree for finding StereoHits with using CDCRecoHit3D as the item type.
     * This implementation uses the HitInZ1Z2 decision algorithm with 2 divisions for each step.
     * This class is only an alias. The real algorithm can be found in SimpleBoxDivisionHoughTree.
     */
    using HitQuadraticLegendre =
      Z1Z2Legendre<std::pair<CDCRecoHit3D, const CDCRLWireHit*>, FirstOfPairInBox<HitInZ1Z2Box>, 2, 2>;
  }
}
