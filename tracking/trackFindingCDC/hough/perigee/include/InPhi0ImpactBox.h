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

    /// Checker if a position is contained in a family of curves over phi0 and impact
    class InPhi0ImpactBox {

    public:
      /// Constructor taking the curler curvature - give a zero greater 0 to activate one arm exclusive finding.
      InPhi0ImpactBox(float curlCurv = NAN) : m_curlCurv(curlCurv) {}

    public:
      /// The box to which this object correspondes.
      typedef Box<DiscretePhi0, ContinuousImpact> HoughBox;

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
        const std::array<DiscretePhi0, 2>& phi0Vec = houghBox.getBounds<DiscretePhi0>();
        const std::array<ContinuousImpact, 2>& impact = houghBox.getBounds<ContinuousImpact>();

        std::array<float, 2> xRot;
        xRot[0] = x * phi0Vec[0]->x() + y * phi0Vec[0]->y();
        xRot[1] = x * phi0Vec[1]->x() + y * phi0Vec[1]->y();

        const bool onlyPositiveArm = 0 < m_curlCurv;
        if (onlyPositiveArm) {
          // Reject hit if it is on the inward going branch but the curvature suggest it is no curler
          if (xRot[0] < 0 and xRot[1] < 0) return INVALID_SIGN;
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

        return SameSignChecker::commonSign(dist);
      }

    private:
      /// Curler curvature - set to value greater zero to activate one arm exclusive finding.
      float m_curlCurv;
    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
