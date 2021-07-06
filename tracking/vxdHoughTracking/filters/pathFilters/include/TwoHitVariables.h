/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/geometry/B2Vector3.h>

#include <cmath>

namespace Belle2 {
  namespace vxdHoughTracking {

    /// Class that allows the calculation of simple variables to check whether a combination of two hits should be used or discarded in tracking.
    class TwoHitVariables {
    public:
      /// Constructor
      /// @param oHit B2Vector3D of the outer hit used for calculating the single variables
      /// @param iHit B2Vector3D of the inner hit used for calculating the single variables
      TwoHitVariables(const B2Vector3D& oHit, const B2Vector3D& iHit) : m_oHit(oHit), m_iHit(iHit)
      {};

      /// Set hits if not given in constructor of if they need to be changed.
      /// @param oHit B2Vector3D of the outer hit used for calculating the single variables
      /// @param iHit B2Vector3D of the inner hit used for calculating the single variables
      void setHits(const B2Vector3D& oHit, const B2Vector3D& iHit)
      {
        m_oHit = oHit;
        m_iHit = iHit;
      }

      /// calculate the cosine of the angle between two vectors in x-y
      double getCosXY()
      {
        return (m_oHit.X() * m_iHit.X() + m_oHit.Y() * m_iHit.Y()) / (m_oHit.Perp() * m_iHit.Perp());
      }

      /// get the difference in z between two vectors
      double get1DZDistance()
      {
        return m_oHit.Z() - m_iHit.Z();
      }

      /// get the squared difference in z between two vectors
      double get1DZDistanceSquared()
      {
        return (m_oHit.Z() - m_iHit.Z()) * (m_oHit.Z() - m_iHit.Z());
      }

      /// get the squared distance between two vectors in x-y
      double get2DXYDistanceSquared()
      {
        return (m_oHit - m_iHit).Perp2();
      }

      /// get the square of the difference of two vectors
      double get3DDistanceSquared()
      {
        return (m_oHit - m_iHit).Mag2();
      }

      /// get an estimate for the slope in R-z, similar to theta
      double getRZSlope()
      {
        double result = atan(
                          sqrt(std::pow(double(m_oHit.X() - m_iHit.X()), 2)
                               + std::pow(double(m_oHit.Y() - m_iHit.Y()), 2)) /
                          double(m_oHit.Z() - m_iHit.Z()));
        // atan also returns negative angles, so map back to [0,Pi] otherwise one get two peaks at +/-Pi/2 for 90 degree angles
        if (result < 0.0) result += M_PI;

        // TODO: check if 0 is a good default return value in the case z_i==z_o!
        return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
      }

      ///  get the time difference of two space points for the u-side measurement
      /// @param outerSpacePoint outer space point used for calculating the time difference in u
      /// @param innerSpacePoint inner space point used for calculating the time difference in u
      double getDistanceInTimeU(const SpacePoint& outerSpacePoint, const SpacePoint& innerSpacePoint)
      {
        return outerSpacePoint.TimeU() - innerSpacePoint.TimeU();
      }

      ///  get the time difference of two space points for the v-side measurement
      /// @param outerSpacePoint outer space point used for calculating the time difference in v
      /// @param innerSpacePoint inner space point used for calculating the time difference in v
      double getDistanceInTimeV(const SpacePoint& outerSpacePoint, const SpacePoint& innerSpacePoint)
      {
        return outerSpacePoint.TimeV() - innerSpacePoint.TimeV();
      }

      /// Set the B-Field value used for pT calculations
      /// @param bfieldZ B-Field value to be used
      void setBFieldZ(const double bfieldZ = 1.5) { m_BFieldZ = bfieldZ; }

    private:
      /// BField along z to estimate pT
      double m_BFieldZ = 1.5;
      /// outermost hit position
      B2Vector3D m_oHit;
      /// innermost hit position
      B2Vector3D m_iHit;

    };

  }
}
