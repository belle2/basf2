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

#include <tracking/trackFindingCDC/hough/boxes/Box.h>
#include <tracking/trackFindingCDC/topology/ILayer.h>

#include <array>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Checker if a position is contained in a family of curves over phi0, impact and curvature
    class InPhi0ImpactCurvBox {

    public:
      /**
       *  Constructor taking the curler curvature
       *  Curlers with high curvature than the curler curvature may obtain hits from both arms
       */
      explicit InPhi0ImpactCurvBox(float curlCurv = NAN)
        : m_curlCurv(curlCurv)
      {}

    public:
      /// The box to which this object correspondes.
      using HoughBox = Box<DiscretePhi0, ContinuousImpact, DiscreteCurv>;

    public:
      /**
       *  Function that gives the sign of the distance from an observed drift circle to the familiy of curves
       *  @returns
       *   * ESign::c_Plus if the drift circle is always on the positive / right site
       *   * ESign::c_Minus means the drift circle is always on the negative / left site
       *   * ESign::c_Zero if the dirft circle lies on any of the curves
       *   * ESign::c_Invalid if the drift cirlce is on the the wrong arm of the curve.
       */
      ESign getDistanceSign(const HoughBox& houghBox,
                            float x,
                            float y,
                            float l,
                            float /*dxdz*/ = 0,
                            float /*dydz*/ = 0,
                            ILayer /*iCLayer*/ = -1) const
      {
        const std::array<DiscretePhi0, 2>& phi0Vec = houghBox.getBounds<DiscretePhi0>();
        const std::array<DiscreteCurv, 2>& curv = houghBox.getBounds<DiscreteCurv>();

        std::array<float, 2> xRot;
        xRot[0] = x * phi0Vec[0]->x() + y * phi0Vec[0]->y();
        xRot[1] = x * phi0Vec[1]->x() + y * phi0Vec[1]->y();

        const bool isNonCurler = (static_cast<float>(curv[1]) <= m_curlCurv and
                                  static_cast<float>(curv[0]) >= -m_curlCurv);
        const bool onlyPositiveArm = isNonCurler;

        if (onlyPositiveArm) {
          // Reject hit if it is on the inward going branch but the curvature suggest it is no curler
          if (xRot[0] < 0 and xRot[1] < 0) return ESign::c_Invalid;
        }

        std::array<float, 2> yRot;
        yRot[0] = -x * phi0Vec[0]->y() + y * phi0Vec[0]->x();
        yRot[1] = -x * phi0Vec[1]->y() + y * phi0Vec[1]->x();

        const std::array<ContinuousImpact, 2>& impact = houghBox.getBounds<ContinuousImpact>();

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
        std::array<ESign, 2> distSign;
        for (int c_Curv = 0; c_Curv < 2; ++c_Curv) {
          std::array<float, 4> dist;
          float curvHalf  = static_cast<float>(curv[c_Curv]) / 2;
          dist[0b00] = - yRotMinusI[0b00] + r2MinusI[0b00] * curvHalf - l;
          dist[0b01] = - yRotMinusI[0b01] + r2MinusI[0b01] * curvHalf - l;
          dist[0b10] = - yRotMinusI[0b10] + r2MinusI[0b10] * curvHalf - l;
          dist[0b11] = - yRotMinusI[0b11] + r2MinusI[0b11] * curvHalf - l;
          distSign[c_Curv] = ESignUtil::common(dist);
        }

        return ESignUtil::common(distSign[0], distSign[1]);

      }

    private:
      /// Curler curvature - set to value greater zero to activate on arm exclusive finding.
      float m_curlCurv;
    };
  }
}
