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
#include <Math/Vector2D.h>
#include <Math/VectorUtil.h>

namespace Belle2 {

  namespace VectorUtil {

    inline bool hasNAN(const ROOT::Math::XYVector& a)
    {
      return std::isnan(a.X()) or std::isnan(a.Y());
    }

    inline bool hasNAN(const ROOT::Math::XYZVector& a)
    {
      return std::isnan(a.X()) or std::isnan(a.Y()) or std::isnan(a.Z());
    }

    inline bool isNull(const ROOT::Math::XYVector& a)
    {
      return a.X() == 0.0 and a.Y() == 0.0;
    }

    inline bool isNull(const ROOT::Math::XYZVector& a)
    {
      return a.X() == 0.0 and a.Y() == 0.0 and a.Z() == 0.0;
    }

    inline ROOT::Math::XYVector average(const ROOT::Math::XYVector& a, const ROOT::Math::XYVector& b)
    {
      if (hasNAN(a)) {
        return b;
      } else if (hasNAN(b)) {
        return a;
      } else {
        return ROOT::Math::XYVector((a.X() + b.X()) / 2.0, (a.Y() + b.Y()) / 2.0);
      }
    }

    inline ROOT::Math::XYVector average(const ROOT::Math::XYVector& a, const ROOT::Math::XYVector& b, const ROOT::Math::XYVector& c)
    {
      if (hasNAN(a)) {
        return average(b, c);
      } else if (hasNAN(b)) {
        return average(a, c);
      } else if (hasNAN(c)) {
        return average(a, b);
      } else {
        return ROOT::Math::XYVector((a.X() + b.X() + c.X()) / 3.0,
                                    (a.Y() + b.Y() + c.Y()) / 3.0);
      }
    }

  }

}
