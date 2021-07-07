/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/perigee/SimpleHitBasedHoughTree.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A simple hough tree taking rl tagable wire hits as smallest units.
    template<class AInBox, size_t ... divisions>
    using SimpleRLTaggedWireHitHoughTree =
      SimpleHitBasedHoughTree<CDCRLWireHit, AInBox, divisions ... >;

  }
}
