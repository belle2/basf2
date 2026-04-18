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

    /// Constructs the average of two vectors
    /** Computes the average of two vectors
     *  If one vector contains NAN the average is the other vector, since the former is not
     * considered a valid value.
     **/
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

    /// Constructs the average of three vectors
    /** Computes the average of three vectors. In case one of the two dimensional vectors contains a NAN,
      *  it is not considered a valid value for the average and is therefore left out.
      *  The average() of the other two vectors is then returned.
      **/
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

    /// Constructs the average of two vectors
    /** Computes the average of two vectors.
     *  If one vector contains NAN the average is the other vector, since the former is not
     *  considered a valid value.
     **/
    inline ROOT::Math::XYZVector average(const ROOT::Math::XYZVector& a, const ROOT::Math::XYZVector& b)
    {
      if (hasNAN(a)) {
        return b;
      } else if (hasNAN(b)) {
        return a;
      } else {
        return ROOT::Math::XYZVector((a.x() + b.x()) / 2.0,
                                     (a.y() + b.y()) / 2.0,
                                     (a.z() + b.z()) / 2.0);
      }
    }

    /// Constructs a vector from a unit coordinate system vector and the coordinates in that
    /// system
    /** Combines a coordinate system vector expressed in laboratory coordinates \n
     *  with the parallel and orthogonal components in the coordinate system \n
     *  to a vector in laboratory coordinates. The coordinate system vector \n
     *  is assumed the unit of the coordinate system */
    inline ROOT::Math::XYVector compose(const ROOT::Math::XYVector& coordinateVec, const double parallelCoor, const double orthoCoor)
    {
      return ROOT::Math::XYVector(coordinateVec.x() * parallelCoor - coordinateVec.y() * orthoCoor,
                                  coordinateVec.y() * parallelCoor + coordinateVec.x() * orthoCoor);
    }

    /// Returns a unit vector colaligned with this
    template<class Vector>
    inline Vector unit(const Vector& a)
    {
      if (isNull(a)) {
        return Vector();
      }
      Vector tmp(a);
      return tmp.Scale(1. / tmp.R());
    }

    /// Constructs a unit vector with azimuth angle equal to phi
    inline ROOT::Math::XYVector Phi(const double phi)
    {
      return std::isnan(phi) ? ROOT::Math::XYVector(0.0, 0.0) : ROOT::Math::XYVector(std::cos(phi), std::sin(phi));
    }

  } // namespace VectorUtil

} // namespace Belle2
