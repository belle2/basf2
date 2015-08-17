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
#include <tracking/trackFindingCDC/hough/perigee/ImpactRep.h>
#include <tracking/trackFindingCDC/hough/perigee/CurvRep.h>
#include <tracking/trackFindingCDC/hough/Box.h>
#include <tracking/trackFindingCDC/hough/SameSignChecker.h>
#include <tracking/trackFindingCDC/topology/ILayerType.h>

#include <array>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Checker if a position is contained in a family of curves over phi0, impact and curvature
    class InPhi0ImpactCurvBox {

    public:
      /** Constructor taking the curler curvature
       *  Curlers with high curvature than the curler curvature may obtain hits from both arms*/
      InPhi0ImpactCurvBox(float curlCurv = NAN) : m_curlCurv(curlCurv) {}

    public:
      /// The box to which this object correspondes.
      typedef Box<DiscretePhi0, ContinuousImpact, DiscreteCurv> HoughBox;

    public:
      /** Function that gives the sign of the distance from an observed drift circle to the familiy of curves
       *  @returns
       *   * PLUS if the drift circle is always on the positive / right site
       *   * MINUS means the drift circle is always on the negative / left site
       *   * ZERO if the dirft circle lies on any of the curves
       *   * INVALID_SIGN if the drift cirlce is on the the wrong arm of the curve.
       */
      inline SignType getDistanceSign(const HoughBox& houghBox,
                                      const float& x,
                                      const float& y,
                                      const float& l,
                                      const float& /*dxdz*/ = 0,
                                      const float& /*dydz*/ = 0,
                                      const ILayerType& /*iCLayer*/ = -1) const
      {
        const std::array<DiscretePhi0, 2>& phi0Vec = houghBox.template getBounds<DiscretePhi0>();

        std::array<float, 2> xRot;
        xRot[0] = x * phi0Vec[0]->x() + y * phi0Vec[0]->y();
        xRot[1] = x * phi0Vec[1]->x() + y * phi0Vec[1]->y();

        // const bool isNonCurler = float(curv[1]) <= m_curlCurv and float(curv[0]) >= -m_curlCurv;
        // const bool onlyPositiveArm = false; //isNonCurler;

        // if (onlyPositiveArm) {
        //   // Reject hit if it is on the inward going branch but the curvature suggest it is no curler
        //   if (xRot[0] < 0 and xRot[1] < 0) return INVALID_SIGN;
        // }

        std::array<float, 2> yRot;
        yRot[0] = -x * phi0Vec[0]->y() + y * phi0Vec[0]->x();
        yRot[1] = -x * phi0Vec[1]->y() + y * phi0Vec[1]->x();

        const std::array<ContinuousImpact, 2>& impact = houghBox.template getBounds<ContinuousImpact>();

        // Using binary notation encoding lower and upper box bounds to fill the flat array.
        std::array<float, 4> yRotMinusI;
        yRotMinusI[0b00] = yRot[0] - impact[0];
        yRotMinusI[0b01] = yRot[0] - impact[1];
        yRotMinusI[0b10] = yRot[1] - impact[0];
        yRotMinusI[0b11] = yRot[1] - impact[1];

        const float l2 = l * l;
        std::array<float, 4> r2MinusI;
        r2MinusI[0b00] = xRot[0] * xRot[0] + yRotMinusI[0b00] * yRotMinusI[0b00] - l2;
        r2MinusI[0b01] = xRot[0] * xRot[0] + yRotMinusI[0b01] * yRotMinusI[0b01] - l2;
        r2MinusI[0b10] = xRot[1] * xRot[1] + yRotMinusI[0b10] * yRotMinusI[0b10] - l2;
        r2MinusI[0b11] = xRot[1] * xRot[1] + yRotMinusI[0b11] * yRotMinusI[0b11] - l2;

        // Using binary notation encoding lower and upper box bounds to fill the flat array.
        const std::array<DiscreteCurv, 2>& curv = houghBox.template getBounds<DiscreteCurv>();

        // std::array<float, 8> dist;
        // dist[0b000] = - yRotMinusI[0b00] + r2MinusI[0b00] * (float(curv[0]) / 2) - l;
        // dist[0b001] = - yRotMinusI[0b00] + r2MinusI[0b00] * (float(curv[0]) / 2) - l;

        // dist[0b010] = - yRotMinusI[0b01] + r2MinusI[0b01] * (float(curv[0]) / 2) - l;
        // dist[0b011] = - yRotMinusI[0b01] + r2MinusI[0b01] * (float(curv[1]) / 2) - l;

        // dist[0b100] = - yRotMinusI[0b10] + r2MinusI[0b10] * (float(curv[0]) / 2) - l;
        // dist[0b101] = - yRotMinusI[0b10] + r2MinusI[0b10] * (float(curv[1]) / 2) - l;

        // dist[0b110] = - yRotMinusI[0b11] + r2MinusI[0b11] * (float(curv[0]) / 2) - l;
        // dist[0b111] = - yRotMinusI[0b11] + r2MinusI[0b11] * (float(curv[1]) / 2) - l;

        // Using binary notation encoding lower and upper box bounds to fill the flat array.
        std::array<SignType, 2> distSign;
        for (int iCurv = 0; iCurv < 2; ++iCurv) {
          std::array<float, 4> dist;
          float curvHalf  = float(curv[iCurv]) / 2;
          dist[0b00] = - yRotMinusI[0b00] + r2MinusI[0b00] * curvHalf - l;
          dist[0b01] = - yRotMinusI[0b01] + r2MinusI[0b01] * curvHalf - l;
          dist[0b10] = - yRotMinusI[0b10] + r2MinusI[0b10] * curvHalf - l;
          dist[0b11] = - yRotMinusI[0b11] + r2MinusI[0b11] * curvHalf - l;
          distSign[iCurv] = SameSignChecker::commonSign(dist);
        }

        // SignType ll =  SameSignChecker::commonSign(dist[0], dist[1]);
        // SignType lu =  SameSignChecker::commonSign(dist[2], dist[3]);

        // SignType ul =  SameSignChecker::commonSign(dist[4], dist[5]);
        // SignType uu =  SameSignChecker::commonSign(dist[6], dist[7]);

        // SignType k  = SameSignChecker::commonSign(ll, lu);
        // SignType u  = SameSignChecker::commonSign(ul, uu);

        //return SameSignChecker::commonSign(k, u);

        return SameSignChecker::commonSign(distSign[0], distSign[1]);

      }

    private:
      /// Curler curvature - set to value greater zero to activate on arm exclusive finding.
      float m_curlCurv;
    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
