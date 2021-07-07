/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/boxes/Box.h>
#include <tracking/trackFindingCDC/topology/ILayer.h>

#include <array>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Checker if a position is contained in a family of curves over phi0
    class InPhi0Box {

    public:
      /// Constructor taking the curler curvature - pass a value greater 0 to activate one arm exclusive finding.
      InPhi0Box(float curlCurv = NAN) : m_curlCurv(std::fabs(curlCurv)) {}

    public:
      /// The box to which this object correspondes.
      using HoughBox = Box<DiscretePhi0>;

    public:
      /** Function that gives the sign of the distance from an observed drift circle to the familiy of curves
       *  @param houghBox
       *  @param x x coordinate of the center of the drift circle
       *  @param y y coordinate of the center of the drift circle
       *  @param l the signed drift length of the drift circle - sign is the right left passage hypotheses
       *  @return
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

        std::array<float, 2> xRot;
        xRot[0] = x * phi0Vec[0]->x() + y * phi0Vec[0]->y();
        xRot[1] = x * phi0Vec[1]->x() + y * phi0Vec[1]->y();

        const bool onlyPositiveArm = 0 < m_curlCurv;
        if (onlyPositiveArm) {
          // Reject hit if it is on the inward going branch but the curvature suggest it is no curler
          if (xRot[0] < 0 and xRot[1] < 0) return ESign::c_Invalid;
        }

        std::array<float, 2> yRotPlusL;
        yRotPlusL[0] = -x * phi0Vec[0]->y() + y * phi0Vec[0]->x() - l;
        yRotPlusL[1] = -x * phi0Vec[1]->y() + y * phi0Vec[1]->x() - l;

        std::array<float, 2> dist;
        dist[0] = - yRotPlusL[0];
        dist[1] = - yRotPlusL[1];

        return ESignUtil::common(dist[0], dist[1]);
      }

    private:
      /// Curler curvature - set a value greater zero to activate one arm exclusive finding.
      float m_curlCurv;
    };
  }
}
