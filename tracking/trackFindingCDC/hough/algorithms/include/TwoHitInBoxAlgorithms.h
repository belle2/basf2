/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Predicate class to check for the containment of items in a hough space part with two algorithms.
     * The resulting weight is added. If one of the two algorithms gives NaN, this particular result is
     * not used (only the other). If both give NaN, the result will also be NaN.
     */
    template<class AHitInBoxAlgorithm, class AnotherHitInBoxAlgorithm>
    class TwoHitInBoxAlgorithm {
    public:
      /// The type of the underlaying HoughBox (copied from the first underlying hit algorithm)
      using HoughBox = typename AHitInBoxAlgorithm::HoughBox;

      /**
       * Returns the sum of the resulting weights of both algorithms (of not NAN).
       */
      template<class AObject>
      Weight operator()(const AObject& object,
                        const HoughBox* box)
      {
        AHitInBoxAlgorithm hitInBoxAlgorithm;
        AnotherHitInBoxAlgorithm anotherHitInBoxAlgorithm;

        const Weight& firstResult = hitInBoxAlgorithm(object, box);
        const Weight& secondResult = anotherHitInBoxAlgorithm(object, box);

        if (std::isnan(firstResult) and std::isnan(secondResult)) {
          return NAN;
        } else if (std::isnan(firstResult)) {
          return secondResult;
        } else if (std::isnan(secondResult)) {
          return firstResult;
        } else {
          return firstResult + secondResult;
        }
      }
    };
  }
}
