/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/numerics/numerics.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Helper class to redirect only the first element in a pair to the child algorithm.
     */
    template<class AHitInBoxAlgorithm>
    class FirstOfPairInBox {
    public:
      /// The type of the underlaying HoughBox (copied from the underlaying hit algorithm)
      typedef typename AHitInBoxAlgorithm::HoughBox HoughBox;

      /// Redirect the first element of a pair to the next algorithm.
      template<class APairObject>
      inline Weight operator()(const APairObject& pairObject,
                               const HoughBox* box)
      {
        AHitInBoxAlgorithm hitInBoxAlgorithm;
        return hitInBoxAlgorithm(pairObject.first, box);
      }
    };
  }
}
