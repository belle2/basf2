/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <Math/Vector3D.h>
#include <TVector3.h>

namespace Belle2 {

  /**
   * Helper function to convert XYZVector to TVector3
   */
  static constexpr auto XYZToTVector = [](const ROOT::Math::XYZVector& a)
  {
    return TVector3(a.X(), a.Y(), a.Z());
  };

  namespace VectorUtil {

    /**
     * Set vector by polar coordinates.
     * @param[out] vector Vector.
     * @param[in]  mag    Magnitude.
     * @param[in]  theta  Polar angle.
     * @param[in]  phi    Azimuthal angle.
     */
    inline void setMagThetaPhi(ROOT::Math::XYZVector& vector,
                               double mag, double theta, double phi)
    {
      const double amag = std::abs(mag);
      const double sinTheta = std::sin(theta);
      const double x = amag * sinTheta * std::cos(phi);
      const double y = amag * sinTheta * std::sin(phi);
      const double z = amag * std::cos(theta);
      vector.SetXYZ(x, y, z);
    }

    /**
     * Return angle in the range [0, 2 * M_PI) that differs from
     * the agrument by 2 * M_PI * n.
     */
    double phi0TwoPi(double angle);

    /**
     * Return angle in the range [-M_PI, M_PI) that differs from
     * the agrument by 2 * M_PI * n.
     */
    double phiMinusPiPi(double angle);

  }

}
