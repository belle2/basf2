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

namespace Belle2 {

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
      vector.SetX(x);
      vector.SetY(y);
      vector.SetZ(z);
    }

  }

}
