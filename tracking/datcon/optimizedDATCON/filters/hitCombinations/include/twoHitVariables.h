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
#include <framework/geometry/B2Vector3.>

#include <cmath>

namespace Belle2 {

  double getCosXY(const B2Vector3D& oHit, const B2Vector3D& iHit) const
  {
    return oHit.Unit() * iHit.Unit();
  }

  double get1DZDistance(const B2Vector3D& oHit, const B2Vector3D& iHit) const
  {
    return oHit.Z() - iHit.Z();
  }

  double get1DZDistanceSquared(const B2Vector3D& oHit, const B2Vector3D& iHit) const
  {
    return (oHit.Z() - iHit.Z()) * (oHit.Z() - iHit.Z());
  }

  double get2DXYDistanceSquared(const B2Vector3D& oHit, const B2Vector3D& iHit) const
  {
//     return (oHit.X() - iHit.X()) * (oHit.X() - iHit.X()) + (oHit.Y() - iHit.Y()) * (oHit.Y() - iHit.Y());
    return (oHit - iHit).Perp2();
  }

// // //   double get3DNormedXYDistance(const B2Vector3D& oHit, const B2Vector3D& iHit) const {
// // //     double result =   (oHit - iHit).Perp2()
// // //       /
// // //       (std::pow(outerHit.X() - innerHit.X() , 2)
// // //         + std::pow(outerHit.Y() - innerHit.Y() , 2)
// // //         + std::pow(outerHit.Z() - innerHit.Z() , 2));
// // //
// // //     return
// // //       (std::isnan(result) || std::isinf(result)) ? 0 : result;
// // //   }

  double get3DDistanceSquared(const B2Vector3D& oHit, const B2Vector3D& iHit) const
  {
//     return  ((oHit.X() - iHit.X()) * (oHit.X() - iHit.X())) +
//             ((oHit.Y() - iHit.Y()) * (oHit.Y() - iHit.Y())) +
//             ((oHit.Z() - iHit.Z()) * (oHit.Z() - iHit.Z()));
    return (oHit - iHit).Mag2();
  }

  double getRZSlope(const B2Vector3D& oHit, const B2Vector3D& iHit) const
  {
    double result = atan(
                      sqrt(std::pow(double(outerHit.X() - innerHit.X()), 2)
                           + std::pow(double(outerHit.Y() - innerHit.Y()), 2)
                          ) / double(outerHit.Z() - innerHit.Z())
                    );
    // atan also returns negative angles, so map back to [0,Pi] otherwise one get two peaks at +/-Pi/2 for 90 degree angles
    if (result < 0.0) result += M_PI;

    // TODO: check if 0 is a good default return value in the case z_i==z_o!
    return (std::isnan(result) || std::isinf(result)) ? double(0) : result;
  }


  double getDistanceInTimeU(const SpacePoint& oHit, const SpacePoint& iHit) const
  {
    return oHit.TimeU() - iHit.TimeU();
  }

  double getDistanceInTimeV(const SpacePoint& oHit, const SpacePoint& iHit) const
  {
    return oHit.TimeV() - iHit.TimeV();
  }


}
