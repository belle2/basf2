/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/axes/StandardAxes.h>
#include <tracking/trackFindingCDC/hough/boxes/Box.h>
#include <tracking/trackFindingCDC/topology/ILayer.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Checker if a position is contained in a family of curves over phi0 and tan lambda
    class InPhi0TanLBox {

    public:
      /// Create a new box with the given curler curvature.
      explicit InPhi0TanLBox(float curlCurv)
        : m_curlCurv(curlCurv)
      {}

    public:
      /// The box to which this object correspondes.
      using HoughBox = Box<DiscretePhi0, ContinuousTanL>;

    public:
      /// Function that gives the sign of the distance from an observed drift circle to the sweeped object.
      ESign getDistanceSign(const HoughBox& houghBox,
                            float x,
                            float y,
                            float l,
                            float dxdz,
                            float dydz,
                            ILayer /*iCLayer*/ = -1) const
      {
        const std::array<DiscretePhi0, 2>& phi0Vec = houghBox.getBounds<DiscretePhi0>();
        const std::array<ContinuousTanL, 2>& tanL = houghBox.getBounds<ContinuousTanL>();

        std::array<float, 2> xRot;
        xRot[0] = x * phi0Vec[0]->x() + y * phi0Vec[0]->y();
        xRot[1] = x * phi0Vec[1]->x() + y * phi0Vec[1]->y();

        std::array<float, 2> yRot;
        yRot[0] = -x * phi0Vec[0]->y() + y * phi0Vec[0]->x();
        yRot[1] = -x * phi0Vec[1]->y() + y * phi0Vec[1]->x();

        std::array<float, 2> dxdzRot;
        dxdzRot[0] = dxdz * phi0Vec[0]->x() + dydz * phi0Vec[0]->y();
        dxdzRot[1] = dxdz * phi0Vec[1]->x() + dydz * phi0Vec[1]->y();

        std::array<float, 2> dydzRot;
        dydzRot[0] = -dxdz * phi0Vec[0]->y() + dydz * phi0Vec[0]->x();
        dydzRot[1] = -dxdz * phi0Vec[1]->y() + dydz * phi0Vec[1]->x();

        /// Two dimensional arc length
        const std::array<float, 2>& s = xRot;

        const bool onlyPositiveArm = 0 < m_curlCurv;
        if (onlyPositiveArm) {
          // Reject hit if it is on the inward going branch but the curvature suggest it is no curler
          if (xRot[0] < 0 and xRot[1] < 0) return ESign::c_Invalid;
        }

        // cppcheck-suppress unreadVariable
        std::array<float, 4> xRotCor;
        xRotCor[0b00] = xRot[0] + dxdzRot[0] * s[0] * static_cast<float>(tanL[0]);
        xRotCor[0b01] = xRot[0] + dxdzRot[0] * s[0] * static_cast<float>(tanL[1]);
        xRotCor[0b10] = xRot[1] + dxdzRot[1] * s[1] * static_cast<float>(tanL[0]);
        xRotCor[0b11] = xRot[1] + dxdzRot[1] * s[1] * static_cast<float>(tanL[1]);

        const std::array<float, 4>& sCor = xRotCor;

        std::array<float, 4> dist;
        dist[0b00] = -(yRot[0] + dydzRot[0] * sCor[0b00] * static_cast<float>(tanL[0]) + l);
        dist[0b01] = -(yRot[0] + dydzRot[0] * sCor[0b01] * static_cast<float>(tanL[1]) + l);
        dist[0b10] = -(yRot[1] + dydzRot[1] * sCor[0b10] * static_cast<float>(tanL[0]) + l);
        dist[0b11] = -(yRot[1] + dydzRot[1] * sCor[0b11] * static_cast<float>(tanL[1]) + l);

        return ESignUtil::common(dist);
      }

    private:
      /// Curler curvature - set to value greater zero to activate one arm exclusive finding.
      float m_curlCurv;
    };
  }
}
