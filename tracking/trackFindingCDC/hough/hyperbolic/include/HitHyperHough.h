/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
     * Hough Tree for finding StereoHits using CDCRecoHit3D as the item type.
     * This implementation uses the HitInHyperBox decision algorithm with 4, 3, and 2 divisions of each axis for each step.
     * Precision is driven by the frist axis.
     * This class is only an alias. The real algorithm can be found in OtherSimpleBoxDivisionHoughTree and HitInHyperBox algorithm.
     */
    using HitHyperHough =
      HyperHough<std::pair<CDCRecoHit3D, const CDCRLWireHit*>, FirstOfPairInBox<HitInHyperBox>, 4, 3, 2>;
  }
}
