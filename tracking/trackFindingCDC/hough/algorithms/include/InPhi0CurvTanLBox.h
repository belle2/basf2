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

#include <tracking/trackFindingCDC/hough/axes/StandardAxes.h>
#include <tracking/trackFindingCDC/hough/boxes/Box.h>
#include <tracking/trackFindingCDC/topology/ILayer.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Checker if a position is contained in a family of curves over phi0 and tan lambda
    class InPhi0CurvTanLBox {

    public:
      /// Create a new box with the given curler curvature.
      explicit InPhi0CurvTanLBox(float curlCurv)
        : m_curlCurv(curlCurv)
      {}

    public:
      /// The box to which this object correspondes.
      using HoughBox = Box<DiscretePhi0, DiscreteCurvWithArcLength2DCache, ContinuousTanL>;

    public:
      /// Function that gives the sign of the distance from an observed drift circle to the sweeped object.
      ESign getDistanceSign(const HoughBox& houghBox,
                            float x,
                            float y,
                            float l,
                            float dxdz,
                            float dydz,
                            ILayer iCLayer = -1) const
      {
        const std::array<DiscretePhi0, 2>& phi0Vec = houghBox.getBounds<DiscretePhi0>();
        const std::array<DiscreteCurvWithArcLength2DCache, 2>& curv =
          houghBox.getBounds<DiscreteCurvWithArcLength2DCache>();
        const std::array<ContinuousTanL, 2>& tanL = houghBox.getBounds<ContinuousTanL>();

        float r2 = x * x + y * y;

        std::array<float, 2> xRot;
        xRot[0] = x * phi0Vec[0]->x() + y * phi0Vec[0]->y();
        xRot[1] = x * phi0Vec[1]->x() + y * phi0Vec[1]->y();

        std::array<float, 2> yRotPlusL;
        yRotPlusL[0] = -x * phi0Vec[0]->y() + y * phi0Vec[0]->x() + l;
        yRotPlusL[1] = -x * phi0Vec[1]->y() + y * phi0Vec[1]->x() + l;

        // std::array<float, 2> dxdzRot;
        // dxdzRot[0] = dxdz * phi0Vec[0]->x() + dydz * phi0Vec[0]->y();
        // dxdzRot[1] = dxdz * phi0Vec[1]->x() + dydz * phi0Vec[1]->y();

        std::array<float, 2> dydzRot;
        dydzRot[0] = -dxdz * phi0Vec[0]->y() + dydz * phi0Vec[0]->x();
        dydzRot[1] = -dxdz * phi0Vec[1]->y() + dydz * phi0Vec[1]->x();

        const bool onlyPositiveArm = 0 < m_curlCurv;
        if (onlyPositiveArm) {
          // Reject hit if it is on the inward going branch but the curvature suggest it is no curler
          if ((xRot[0] < 0) and (xRot[1] < 0)) return ESign::c_Invalid;
        }

        /// Two dimensional arc length
        std::array<float, 2> s;
        s[0] = curv[0]->getArcLength2D(iCLayer, (xRot[0] < 0) and (xRot[1] < 0));
        s[1] = curv[1]->getArcLength2D(iCLayer, (xRot[0] < 0) and (xRot[1] < 0));

        std::array<float, 2> r2TimesHalfCurv;
        r2 -= l * l;
        r2TimesHalfCurv[0] = r2 * (static_cast<float>(*(curv[0])) / 2.0);
        r2TimesHalfCurv[1] = r2 * (static_cast<float>(*(curv[1])) / 2.0);

        // std::array<float, 4> xRotCor;
        // xRotCor[0b00] = xRot[0] + dxdzRot[0] * s[0] * static_cast<float>(tanL[0]);
        // xRotCor[0b01] = xRot[0] + dxdzRot[0] * s[0] * static_cast<float>(tanL[1]);
        // xRotCor[0b10] = xRot[1] + dxdzRot[1] * s[1] * static_cast<float>(tanL[0]);
        // xRotCor[0b11] = xRot[1] + dxdzRot[1] * s[1] * static_cast<float>(tanL[1]);

        // const std::array<float, 4>& sCor = xRotCor;

        std::array<float, 4> z;
        z[0b00] = s[0] * static_cast<float>(tanL[0]);
        z[0b01] = s[1] * static_cast<float>(tanL[0]);
        z[0b10] = s[0] * static_cast<float>(tanL[1]);
        z[0b11] = s[1] * static_cast<float>(tanL[1]);

        // Using binary notation encoding lower and upper box bounds to fill the flat array.
        std::array<float, 8> dist;
        dist[0b000] = r2TimesHalfCurv[0] - yRotPlusL[0] - dydzRot[0] * z[0b00];
        dist[0b001] = r2TimesHalfCurv[0] - yRotPlusL[1] - dydzRot[1] * z[0b00];
        dist[0b010] = r2TimesHalfCurv[1] - yRotPlusL[0] - dydzRot[0] * z[0b01];
        dist[0b011] = r2TimesHalfCurv[1] - yRotPlusL[1] - dydzRot[1] * z[0b01];

        dist[0b100] = r2TimesHalfCurv[0] - yRotPlusL[0] - dydzRot[0] * z[0b10];
        dist[0b101] = r2TimesHalfCurv[0] - yRotPlusL[1] - dydzRot[1] * z[0b10];
        dist[0b110] = r2TimesHalfCurv[1] - yRotPlusL[0] - dydzRot[0] * z[0b11];
        dist[0b111] = r2TimesHalfCurv[1] - yRotPlusL[1] - dydzRot[1] * z[0b11];

        return ESignUtil::common(dist);
      }

    private:
      /// Curler curvature - set to value greater zero to activate one arm exclusive finding.
      float m_curlCurv;
    };
  }
}
