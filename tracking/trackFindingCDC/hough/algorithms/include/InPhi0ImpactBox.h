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

    /// Checker if a position is contained in a family of curves over phi0 and impact
    class InPhi0ImpactBox {

    public:
      /// Constructor taking the curler curvature - give a zero greater 0 to activate one arm exclusive finding.
      explicit InPhi0ImpactBox(float curlCurv = NAN)
        : m_curlCurv(curlCurv)
      {}

    public:
      /// The box to which this object corresponds.
      using HoughBox = Box<DiscretePhi0, ContinuousImpact>;

    public:
      /**
       *  Function that gives the sign of the distance from an observed drift circle to the family of curves
       *  @returns
       *   * ESign::c_Plus if the drift circle is always on the positive / right site
       *   * ESign::c_Minus means the drift circle is always on the negative / left site
       *   * ESign::c_Zero if the drift circle lies on any of the curves
       *   * ESign::c_Invalid if the drift circle is on the wrong arm of the curve.
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
        const std::array<ContinuousImpact, 2>& impact = houghBox.getBounds<ContinuousImpact>();

        std::array<float, 2> xRot;
        xRot[0] = x * phi0Vec[0]->x() + y * phi0Vec[0]->y();
        xRot[1] = x * phi0Vec[1]->x() + y * phi0Vec[1]->y();

        const bool onlyPositiveArm = 0 < m_curlCurv;
        if (onlyPositiveArm) {
          // Reject hit if it is on the inward going branch but the curvature suggest it is no curler
          if (xRot[0] < 0 and xRot[1] < 0) return ESign::c_Invalid;
        }

        std::array<float, 2> yRotPlusL;
        yRotPlusL[0] = -x * phi0Vec[0]->y() + y * phi0Vec[0]->x() + l;
        yRotPlusL[1] = -x * phi0Vec[1]->y() + y * phi0Vec[1]->x() + l;

        // Using binary notation encoding lower and upper box bounds to fill the flat array.
        std::array<float, 4> dist;
        dist[0b00] = yRotPlusL[0] - static_cast<float>(impact[0]);
        dist[0b10] = yRotPlusL[1] - static_cast<float>(impact[0]);
        dist[0b01] = yRotPlusL[0] - static_cast<float>(impact[1]);
        dist[0b11] = yRotPlusL[1] - static_cast<float>(impact[1]);

        return ESignUtil::common(dist);
      }

    private:
      /// Curler curvature - set to value greater zero to activate one arm exclusive finding.
      float m_curlCurv;
    };
  }
}
