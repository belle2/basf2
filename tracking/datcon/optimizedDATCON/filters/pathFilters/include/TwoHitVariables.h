/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2021 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Wessel                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/geometry/B2Vector3.h>

#include <cmath>

namespace Belle2 {

  class TwoHitVariables {
  public:
    TwoHitVariables() : m_oHit(0., 0., 0.), m_iHit(0., 0., 0.)
    {};

    TwoHitVariables(const B2Vector3D& oHit, const B2Vector3D& iHit) : m_oHit(oHit), m_iHit(iHit)
    {};

    double getCosXY()
    {
      return m_oHit.Unit() * m_iHit.Unit();
    }

    double get1DZDistance()
    {
      return m_oHit.Z() - m_iHit.Z();
    }

    double get1DZDistanceSquared()
    {
      return (m_oHit.Z() - m_iHit.Z()) * (m_oHit.Z() - m_iHit.Z());
    }

    double get2DXYDistanceSquared()
    {
      return (m_oHit - m_iHit).Perp2();
    }

    double get3DDistanceSquared()
    {
      return (m_oHit - m_iHit).Mag2();
    }

    double getRZSlope()
    {
      double result = atan(
                        sqrt(std::pow(double(m_oHit.X() - m_iHit.X()), 2)
                             + std::pow(double(m_oHit.Y() - m_iHit.Y()), 2)
                            ) / double(m_oHit.Z() - m_iHit.Z())
                      );
      // atan also returns negative angles, so map back to [0,Pi] otherwise one get two peaks at +/-Pi/2 for 90 degree angles
      if (result < 0.0) result += M_PI;

      // TODO: check if 0 is a good default return value in the case z_i==z_o!
      return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
    }


    double getDistanceInTimeU(const SpacePoint& outerSpacePoint, const SpacePoint& innerSpacePoint)
    {
      return outerSpacePoint.TimeU() - innerSpacePoint.TimeU();
    }

    double getDistanceInTimeV(const SpacePoint& outerSpacePoint, const SpacePoint& innerSpacePoint)
    {
      return outerSpacePoint.TimeV() - innerSpacePoint.TimeV();
    }

    void setBFieldZ(const double bfieldZ = 1.5) { m_BFieldZ = bfieldZ; }

  private:
    double m_BFieldZ = 1.5;
    B2Vector3D m_oHit;
    B2Vector3D m_iHit;

  };
}
