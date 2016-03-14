/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/perigee/ImpactRep.h>
#include <tracking/trackFindingCDC/hough/boxes/SweepBox.h>
#include <tracking/trackFindingCDC/hough/SameSignChecker.h>
#include <tracking/trackFindingCDC/topology/ILayer.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Takes a basic object and sweeps it by some d0 range in the y direction.
     *  The sweep in phi0 should be handled before
     */
    template<class T, class AImpact = ContinuousImpact>
    class AImpactSweeped : public T {

    public:
      /// Using the constructor of the base class.
      using T::T;

    public:
      /// The box to which this object correspondes.
      typedef SweepBox<AImpact, typename T::HoughBox> HoughBox;

      /// The hough box without the sweep in d0
      typedef typename T::HoughBox SubordinaryHoughBox;

    public:
      /** Function that gives the sign of the distance from an observed drift circle to the sweeped object
       */
      inline ESign getDistanceSign(const HoughBox& houghBox,
                                   const float& x,
                                   const float& y,
                                   const float& signedDriftLength,
                                   const float& dxdz = 0,
                                   const float& dydz = 0,
                                   ILayer iCLayer = -1) const
      {
        const float& lowerImpact(houghBox.template getLowerBound<AImpact>());
        const float& upperImpact(houghBox.template getUpperBound<AImpact>());
        const SubordinaryHoughBox& subordinaryHoughBox = houghBox.getSubordinaryBox();

        const float lowerY = y - lowerImpact;
        const ESign lowerDistSign = T::getDistanceSign(subordinaryHoughBox,
                                                       x, lowerY,
                                                       signedDriftLength,
                                                       dxdz, dydz,
                                                       iCLayer);

        const float upperY = y - upperImpact;
        const ESign upperDistSign = T::getDistanceSign(subordinaryHoughBox,
                                                       x, upperY,
                                                       signedDriftLength,
                                                       dxdz, dydz,
                                                       iCLayer);

        return ESignUtil::common(lowerDistSign, upperDistSign);
      }
    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
