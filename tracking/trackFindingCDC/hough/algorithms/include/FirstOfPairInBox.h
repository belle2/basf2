/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Helper class to redirect only the first element in a pair to the child algorithm.
     */
    template<class AHitInBoxAlgorithm>
    class FirstOfPairInBox {
    public:
      /// The type of the underlying HoughBox (copied from the underlying hit algorithm)
      using HoughBox = typename AHitInBoxAlgorithm::HoughBox;
      /// The hit-finding algorithm
      using BoxAlgorithm = AHitInBoxAlgorithm;

      /// Redirect the first element of a pair to the next algorithm.
      template<class APairObject>
      Weight operator()(const APairObject& pairObject,
                        const HoughBox* box)
      {
        AHitInBoxAlgorithm hitInBoxAlgorithm;
        return hitInBoxAlgorithm(pairObject.first, box);
      }
    };
  }
}
