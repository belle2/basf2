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

#include <tracking/trackFindingCDC/hough/perigee/Phi0Rep.h>
#include <tracking/trackFindingCDC/hough/perigee/CurvRep.h>
#include <tracking/trackFindingCDC/hough/boxes/SweepBox.h>
#include <tracking/trackFindingCDC/hough/baseelements/SameSignChecker.h>
#include <tracking/trackFindingCDC/topology/ILayer.h>

namespace Belle2 {
  namespace TrackFindingCDC {


    /// Takes a basic object and sweeps it by some angle phi0 range a around the origin
    template<class T, class APhi0 = DiscretePhi0>
    class APhi0Sweeped : public T {

    public:
      /// Using the constructor of the base class.
      using T::T;

    public:
      /// The box to which this object correspondes.
      using HoughBox = SweepBox<APhi0, typename T::HoughBox>;

      /// The hough box without the sweep in phi0
      using SubordinaryHoughBox = typename T::HoughBox;

      /// Constructor taking the curling curvature.
      APhi0Sweeped(float curlCurv) : m_curlCurv(curlCurv) {}

    public:
      /** Function that gives the sign of the distance from an observed drift circle to the sweeped object
       */
      ESign getDistanceSign(const HoughBox& houghBox,
                            float x,
                            float y,
                            float signedDriftLength,
                            float dxdz = 0,
                            float dydz = 0,
                            ILayer iCLayer = -1) const
      {
        const Vector2D& lowerPhi0Vec(houghBox.template getLowerBound<APhi0>());
        const Vector2D& upperPhi0Vec(houghBox.template getUpperBound<APhi0>());
        const SubordinaryHoughBox& subordinaryHoughBox = houghBox.getSubordinaryBox();

        const float lowerX = x * lowerPhi0Vec.x() + y * lowerPhi0Vec.y();
        const float upperX = x * upperPhi0Vec.x() + y * upperPhi0Vec.y();

        float lowerCurv(getLowerCurv(houghBox));
        float upperCurv(getUpperCurv(houghBox));

        const bool lowerIsNonCurler = fabs(lowerCurv) < m_curlCurv;
        const bool upperIsNonCurler = fabs(upperCurv) < m_curlCurv;

        if (lowerIsNonCurler and upperIsNonCurler) {
          if (not(lowerX >= 0) and not(upperX >= 0)) return ESign::c_Invalid;
        }

        const float lowerY = -x * lowerPhi0Vec.y() + y * lowerPhi0Vec.x();
        const float upperY = -x * upperPhi0Vec.y() + y * upperPhi0Vec.x();

        const float upperDXDZ = dxdz * upperPhi0Vec.x() + dydz * upperPhi0Vec.y();
        const float lowerDXDZ = dxdz * lowerPhi0Vec.x() + dydz * lowerPhi0Vec.y();


        const float lowerDYDZ = -dxdz * lowerPhi0Vec.y() + dydz * lowerPhi0Vec.x();
        const float upperDYDZ = -dxdz * upperPhi0Vec.y() + dydz * upperPhi0Vec.x();

        const ESign lowerDistSign = T::getDistanceSign(subordinaryHoughBox,
                                                       lowerX, lowerY,
                                                       signedDriftLength,
                                                       lowerDXDZ, lowerDYDZ.
                                                       iCLayer);

        const ESign upperDistSign = T::getDistanceSign(subordinaryHoughBox,
                                                       upperX, upperY,
                                                       signedDriftLength,
                                                       upperDXDZ, upperDYDZ,
                                                       iCLayer);

        return ESignUtil::common(lowerDistSign, upperDistSign);
      }

    private:
      /// The curvature above which the trajectory is considered a curler.
      float m_curlCurv = NAN;

    };
  }
}
