/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackingUtilities/numerics/EForwardBackward.h>
#include <tracking/trackingUtilities/numerics/ERightLeft.h>
#include <tracking/trackingUtilities/numerics/ERotation.h>
#include <tracking/trackingUtilities/numerics/ESign.h>

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
      // Somehow, the Cartesion2D vector knows Scale, but DisplacementVector2D does not, and as DisplacementVector2D<Cartesion2D.....
      // So can't use Scale, and perform plain math operations instead... ¯\_(ツ)_/¯
      // return tmp.Scale(1. / tmp.R());
      return tmp / tmp.R();
    }

    /// Constructs a unit vector with azimuth angle equal to phi
    inline ROOT::Math::XYVector Phi(const double phi)
    {
      return std::isnan(phi) ? ROOT::Math::XYVector(0.0, 0.0) : ROOT::Math::XYVector(std::cos(phi), std::sin(phi));
    }

    /// Calculated the two dimensional cross product.
    inline double Cross(const ROOT::Math::XYVector& lhs, const ROOT::Math::XYVector& rhs)
    {
      return lhs.X() * rhs.Y() - lhs.Y() * rhs.X();
    }

    /// Calculated the two dimensional cross product.
    inline ROOT::Math::XYVector Orthogonal(const ROOT::Math::XYVector& a)
    {
      return ROOT::Math::XYVector(-a.Y(), a.X());
    }

    /// Calculated the two dimensional cross product.
    inline ROOT::Math::XYVector Orthogonal(const ROOT::Math::XYVector& a, const TrackingUtilities::ERotation ccwInfo)
    {
      return isValid(ccwInfo) ? ROOT::Math::XYVector(-static_cast<double>(ccwInfo) * a.Y(),
                                                     static_cast<double>(ccwInfo) * a.X()) : ROOT::Math::XYVector();
    }

    /// Calculates the distance of this point to the rhs
    inline double Distance(const ROOT::Math::XYVector& from, const ROOT::Math::XYVector& to = ROOT::Math::XYVector(0.0, 0.0))
    {
      return (from - to).R();
    }

    /// Calculates the distance of this point to the rhs
    inline double Distance(const ROOT::Math::XYZVector& from, const ROOT::Math::XYZVector& to = ROOT::Math::XYZVector(0.0, 0.0, 0.0))
    {
      return (from - to).R();
    }

    /** This assumes the given vector relativeTo to be of unit length and avoids \n
     *  a costly computation of the vector R()
     *  The previous implementation was
     *     relativTo.Dot(*this)
     *  Setting v1 = *this
     */
    template<class Vector>
    inline double unnormalizedParallelComp(const Vector& v1, const Vector& relativTo)
    {
      return relativTo.Dot(v1);
    }

    /// Calculates the component orthogonal to the given vector
    /** The orthogonal component is the component parallel to relativeTo.orthogonal()
     *  The previous implementation was
     *    relativeTo.cross(*this) / relativTo.R()
     *  with v1 = *this
     */
    template<class Vector>
    inline double orthogonalComp(const Vector& v1, const Vector& relativTo)
    {
      return Cross(relativTo, v1) / relativTo.R();
    }

    /// Same as orthogonalComp() but assumes the given vector to be of unit length
    /** This assumes the given vector relativeTo to be of unit length and avoids \n
     *  a costly computation of the vector R()
     *  The previous implementation was
     *    relativeTo.cross(*this)
     *  with v1 = *this
     */
    inline double unnormalizedOrthogonalComp(const ROOT::Math::XYVector& v1, const ROOT::Math::XYVector& relativTo)
    {
      return Cross(relativTo, v1);
    }

    /** Returns a transformed vector version rotated  by the given vector.
     *  The rotated coordinates are such that the given phiVec becomes the new x axes.
     *  @param phiVec *Unit* vector marking the x axes of the new rotated coordinate system*/
    inline ROOT::Math::XYVector passiveRotatedBy(const ROOT::Math::XYVector& v1, const ROOT::Math::XYVector& phiVec)
    {
      return ROOT::Math::XYVector(unnormalizedParallelComp(v1, phiVec), unnormalizedOrthogonalComp(v1, phiVec));
    }

    /// Indicates if the given vector rhs is more left or more right if you looked in the direction of
    /// this vector toCheck.
    inline TrackingUtilities::ERightLeft isRightOrLeftOf(const ROOT::Math::XYVector& toCheck, const ROOT::Math::XYVector& rhs)
    {
      return static_cast<TrackingUtilities::ERightLeft>(-TrackingUtilities::sign(unnormalizedOrthogonalComp(toCheck, rhs)));
    }

    /// Indicates if the given vector rhs is more left if you looked in the direction of this vector .
    inline bool isLeftOf(const ROOT::Math::XYVector& toCheck, const ROOT::Math::XYVector& rhs)
    {
      return isRightOrLeftOf(toCheck, rhs) == TrackingUtilities::ERightLeft::c_Left;
    }

    /// Indicates if the given vector rhs is more right if you looked in the direction of this vector toCheck.
    inline bool isRightOf(const ROOT::Math::XYVector& toCheck, const ROOT::Math::XYVector& rhs)
    {
      return isRightOrLeftOf(toCheck, rhs) == TrackingUtilities::ERightLeft::c_Right;
    }

    /// Check if three values have the same sign.
    inline bool sameSign(float n1, float n2, float n3)
    {
      return ((n1 > 0 and n2 > 0 and n3 > 0) or (n1 < 0 and n2 < 0 and n3 < 0));
    }

    /** Checks if this vector is between two other vectors
     *  Between means here that when rotating the lower vector (first argument)
     *  mathematically positively it becomes coaligned with this vector before
     *  it becomes coalgined with the other vector.
     */
    inline bool isBetween(const ROOT::Math::XYVector& toCheck, const ROOT::Math::XYVector& lower, const ROOT::Math::XYVector& upper)
    {
      // Set up a linear (nonorthogonal) transformation that maps
      // lower -> (1, 0)
      // upper -> (0, 1)
      // Check whether this transformation is orientation conserving
      // If yes this vector must lie in the first quadrant to be between lower and upper
      // If no it must lie in some other quadrant.
      const double det = Cross(lower, upper);
      if (det == 0) {
        // lower and upper are coaligned
        return isRightOf(toCheck, lower) and isLeftOf(toCheck, upper);
      } else {
        const bool flipsOrientation = det < 0;
        const double transformedX = Cross(toCheck, upper);
        const double transformedY = -Cross(toCheck, lower);
        bool inFirstQuadrant = sameSign(det, transformedX, transformedY);
        if (flipsOrientation) {
          inFirstQuadrant = not inFirstQuadrant;
        }
        return inFirstQuadrant;
      }
    }

    /// Indicates if the given vector is more coaligned or reverse if you looked in the direction
    /// of this vector.
    inline TrackingUtilities::EForwardBackward isForwardOrBackwardOf(const ROOT::Math::XYVector& toCheck,
        const ROOT::Math::XYVector& rhs)
    {
      return static_cast<TrackingUtilities::EForwardBackward>(TrackingUtilities::sign(unnormalizedParallelComp(toCheck, rhs)));
    }

    /// Total ordering based on cylindrical radius first and azimuth angle second
    /** Total order achieving a absolute lower bound Vector2D(0.0, 0.0). By first taking the
     * cylindrical radius \n
     *  for comparison the null vector is smaller than all other possible \n
     *  vectors. Secondly the azimuth angle is considered to have a total ordering \n
     *  for all vectors.\n
     */
    inline bool smaller(const ROOT::Math::XYVector& lhs, const ROOT::Math::XYVector& rhs)
    {
      return lhs.Mag2() < rhs.Mag2() or
             (lhs.Mag2() == rhs.Mag2() and (lhs.Phi() < rhs.Phi()));
    }

    /// Total ordering based on cylindrical radius first the z component second and azimuth angle
    /// third.
    /** Total order achieving a lower bound Vector3D(0.0, 0.0, 0.0). By first taking the norm \n
     *  for comparison the null vector is smaller than all other possible \n
     *  vectors. Secondly the polar angle theta ( equivalently z ) and finally the azimuth \n
     *  angle phi is considered to have a total ordering for all vectors. \n
     *  Note does not commute with the projection to xy space.
     **/
    inline bool smaller(const ROOT::Math::XYZVector& lhs, const ROOT::Math::XYZVector& rhs)
    {
      return lhs.R() < rhs.R() or (lhs.R() == rhs.R() and
                                   (lhs.z() < rhs.z() or (lhs.z() == rhs.z() and (lhs.Phi() < rhs.Phi()))));
    }

    /// Get 2D Vector from 3D Vector
    inline ROOT::Math::XYVector get2DVector(const ROOT::Math::XYZVector& a)
    {
      return ROOT::Math::XYVector(a.X(), a.Y());
    }

  } // namespace VectorUtil

} // namespace Belle2