/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Predicate class to check for the containment of items in a rangeobject in a hough space part.
     *  Note this part this code defines the performance of
     *  the search in the hough plain quite significantly and there is probably room for improvement.
     */
    template<class AHitInBoxAlgorithm>
    class RangeInBox {
    public:
      /// The type of the underlaying HoughBox (copied from the udnerlaying hit algorithm)
      using HoughBox = typename AHitInBoxAlgorithm::HoughBox;

      /**  When called, it goes through all items in the range object (e.g. a CDCSegment) and checks the
       *  HitInBox algorithm passed as a template argument. If there is a certain amount of items of the range
       *  in the box, it returns the number of items as a weight, otherwise it returns NAN;
       */
      template<class ARangeObject>
      Weight operator()(const ARangeObject& rangeObject,
                        const HoughBox* box)
      {
        AHitInBoxAlgorithm hitInBoxAlgorithm;
        double sumOfWeights = 0;
        double numberOfPassedItems = 0;
        for (const auto& item : rangeObject) {
          const double hitWeight = hitInBoxAlgorithm(item, box);
          if (not std::isnan(hitWeight)) {
            numberOfPassedItems++;
            sumOfWeights += hitWeight;
          }
        }

        if (numberOfPassedItems > minimalRatio * static_cast<double>(rangeObject.size())) {
          return sumOfWeights;
        } else {
          return NAN;
        }
      }

      /// The minimal percentage of items of a given range that must belong to the box the be called a hit.
      const double minimalRatio = 0.6;
    };
  }
}
