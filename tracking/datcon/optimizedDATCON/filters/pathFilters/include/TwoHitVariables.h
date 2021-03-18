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

  private:
    B2Vector3D m_oHit;
    B2Vector3D m_iHit;

  };
}


// namespace Belle2 {
//
//   static double getCosXY(const B2Vector3D& oHit, const B2Vector3D& iHit) {
//     return oHit.Unit() * iHit.Unit();
//   }
//
//   static double get1DZDistance(const B2Vector3D& oHit, const B2Vector3D& iHit) {
//     return oHit.Z() - iHit.Z();
//   }
//
//   static double get1DZDistanceSquared(const B2Vector3D& oHit, const B2Vector3D& iHit) {
//     return (oHit.Z() - iHit.Z()) * (oHit.Z() - iHit.Z());
//   }
//
//   static double get2DXYDistanceSquared(const B2Vector3D& oHit, const B2Vector3D& iHit) {
// //     return (oHit.X() - iHit.X()) * (oHit.X() - iHit.X()) + (oHit.Y() - iHit.Y()) * (oHit.Y() - iHit.Y());
//     return (oHit - iHit).Perp2();
//   }
//
// // // //   double get3DNormedXYDistance(const B2Vector3D& oHit, const B2Vector3D& iHit) {
// // // //     double result =   (oHit - iHit).Perp2()
// // // //       /
// // // //       (std::pow(oHit.X() - iHit.X() , 2)
// // // //         + std::pow(oHit.Y() - iHit.Y() , 2)
// // // //         + std::pow(oHit.Z() - iHit.Z() , 2));
// // // //
// // // //     return
// // // //       (std::isnan(result) || std::isinf(result)) ? 0 : result;
// // // //   }
//
//   static double get3DDistanceSquared(const B2Vector3D& oHit, const B2Vector3D& iHit) {
// //     return  ((oHit.X() - iHit.X()) * (oHit.X() - iHit.X())) +
// //             ((oHit.Y() - iHit.Y()) * (oHit.Y() - iHit.Y())) +
// //             ((oHit.Z() - iHit.Z()) * (oHit.Z() - iHit.Z()));
//     return (oHit - iHit).Mag2();
//   }
//
//   static double getRZSlope(const B2Vector3D& oHit, const B2Vector3D& iHit) {
//     double result = atan(
//                       sqrt(std::pow(double(oHit.X() - iHit.X()), 2)
//                            + std::pow(double(oHit.Y() - iHit.Y()), 2)
//                           ) / double(oHit.Z() - iHit.Z())
//                     );
//     // atan also returns negative angles, so map back to [0,Pi] otherwise one get two peaks at +/-Pi/2 for 90 degree angles
//     if (result < 0.0) result += M_PI;
//
//     // TODO: check if 0 is a good default return value in the case z_i==z_o!
//     return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
//   }
//
//
//   static double getDistanceInTimeU(const SpacePoint& oHit, const SpacePoint& iHit) {
//     return oHit.TimeU() - iHit.TimeU();
//   }
//
//   static double getDistanceInTimeV(const SpacePoint& oHit, const SpacePoint& iHit) {
//     return oHit.TimeV() - iHit.TimeV();
//   }
// }
