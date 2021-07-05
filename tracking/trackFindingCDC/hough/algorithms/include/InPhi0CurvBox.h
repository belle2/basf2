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
#include <tracking/trackFindingCDC/hough/boxes/Box.h>
#include <tracking/trackFindingCDC/topology/ILayer.h>

#include <array>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Checker if a position is contained in a family of curves over phi0 and curvature
    class InPhi0CurvBox {

    public:
      /**
       *  Constructor taking the curler curvature
       *  Curlers with high curvature than the curler curvature may obtain hits from both arms
       */
      explicit InPhi0CurvBox(float curlCurv = NAN) : m_curlCurv(curlCurv) {}

    public:
      /// The box to which this object correspondes.
      using HoughBox = Box<DiscretePhi0, DiscreteCurv>;

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

        const bool isNonCurler = static_cast<float>(curv[1]) <= m_curlCurv and static_cast<float>(curv[0]) >= -m_curlCurv;
        const bool onlyPositiveArm = isNonCurler;
        if (onlyPositiveArm) {
          // Reject hit if it is on the inward going branch but the curvature suggest it is no curler
          if (xRot[0] < 0 and xRot[1] < 0) return ESign::c_Invalid;
        }

        std::array<float, 2> yRotPlusL;
        yRotPlusL[0] = -x * phi0Vec[0]->y() + y * phi0Vec[0]->x() + l;
        yRotPlusL[1] = -x * phi0Vec[1]->y() + y * phi0Vec[1]->x() + l;

        const float r2 =  x * x + y * y - l * l;
        std::array<float, 2> r2TimesHalfCurv;
        r2TimesHalfCurv[0] = r2 * (static_cast<float>(curv[0]) / 2.0);
        r2TimesHalfCurv[1] = r2 * (static_cast<float>(curv[1]) / 2.0);

        // Using binary notation encoding lower and upper box bounds to fill the flat array.
        std::array<float, 4> dist;

        dist[0b00] = r2TimesHalfCurv[0] - yRotPlusL[0];
        dist[0b10] = r2TimesHalfCurv[0] - yRotPlusL[1];

        dist[0b01] = r2TimesHalfCurv[1] - yRotPlusL[0];
        dist[0b11] = r2TimesHalfCurv[1] - yRotPlusL[1];

        return ESignUtil::common(dist);
      }

    private:
      /// Curler curvature - set to value greater zero to activate on arm exclusive finding.
      float m_curlCurv;
    };
  }
}
