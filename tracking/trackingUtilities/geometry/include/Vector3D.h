/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/geometry/Vector2D.h>
#include <tracking/trackingUtilities/numerics/Quadratic.h>

#include <framework/geometry/B2Vector3.h>
#include <tracking/trackingUtilities/geometry/VectorUtil.h>
#include <framework/geometry/VectorUtil.h>
#include <Math/Vector3D.h>

#include <string>
#include <iosfwd>
#include <cmath>

class TVector3;

namespace Belle2 {
  namespace TrackingUtilities {

    /// A three dimensional vector
    /** A three dimensional vector which is equipped with the expected vector methods. \n
     *  Also this vector can be passed to functions where a TVector3 is expected syntactically.
     */
    class Vector3D {

    public:
      /// Default constructor for ROOT compatibility.
      Vector3D()
        : m_xy(0.0, 0.0)
        , m_z(0.0)
      {
      }

      /// Constructor translating from a TVector3 instance
      explicit Vector3D(const TVector3& tVector3);

      /// Constructor translating from a B2Vector3 instance
      explicit Vector3D(const B2Vector3D& b2Vector3);

      /// Constructor translating from a ROOT::Math::XYZVector instance
      Vector3D(const ROOT::Math::XYZVector& xyzVector);

      /// Constructor from three coordinates
      Vector3D(double x, double y, double z)
        : m_xy(x, y)
        , m_z(z)
      {
      }

      /// Constructor augmeting a Vector2D to a Vector3D setting z to zero
      explicit Vector3D(const Vector2D& xy)
        : m_xy(xy)
        , m_z(0.0)
      {
      }

      /// Constructor augmeting a Vector2D to a Vector3D setting z explicitly
      Vector3D(const Vector2D& xy, double z)
        : m_xy(xy)
        , m_z(z)
      {
      }

      /// Assignment translating from a TVector3 instance
      Vector3D& operator=(const TVector3& tVector3);

      /// Assignment translating from a B2Vector3 instance
      Vector3D& operator=(const B2Vector3D& b2Vector3);

      /// Assignment translating from a ROOT::Math::XYZVector instance
      Vector3D& operator=(const ROOT::Math::XYZVector& xyzVector);

      /// Casting the back to TVector3 seamlessly
      operator const TVector3() const;

      /// Casting the back to B2Vector3 seamlessly
      operator const B2Vector3D() const;

      /// Casting the back to ROOT::Math::XYZVector seamlessly
      operator const ROOT::Math::XYZVector() const;

      /// Equality comparison with all three coordinates
      bool operator==(const Vector3D& rhs) const
      {
        return x() == rhs.x() and y() == rhs.y() and z() == rhs.z();
      }

      /// Total ordering based on cylindrical radius first the z component second and azimuth angle
      /// third.
      /** Total order achieving a lower bound Vector3D(0.0, 0.0, 0.0). By first taking the norm \n
       *  for comparison the null vector is smaller than all other possible \n
       *  vectors. Secondly the polar angle theta ( equivalently z ) and finally the azimuth \n
       *  angle phi is considered to have a total ordering for all vectors. \n
       *  Note does not commute with the projection to xy space.
       **/
      bool operator<(const Vector3D& rhs) const
      {
        return R() < rhs.R() or (R() == rhs.R() and
                                 (z() < rhs.z() or (z() == rhs.z() and (phi() < rhs.phi()))));
      }

      /// Calculates the three dimensional dot product, ROOT::Math compatible
      double Dot(const Vector3D& rhs) const
      {
        return x() * rhs.x() + y() * rhs.y() + z() * rhs.z();
      }

      /// Calculates the two dimensional dot product in xy projection.
      double dotXY(const Vector3D& rhs) const
      {
        return x() * rhs.x() + y() * rhs.y();
      }

      /// Calculated the three dimensional cross product.
      Vector3D cross(const Vector3D& rhs) const
      {
        return Vector3D(y() * rhs.z() - z() * rhs.y(),
                        z() * rhs.x() - x() * rhs.z(),
                        x() * rhs.y() - y() * rhs.x());
      }

      /// Calculates the two dimensional cross product in xy projection.
      double crossXY(const Vector3D& rhs) const
      {
        return VectorUtil::Cross(xy(), rhs.xy());
      }

      /// Calculates the two dimensional cross product in xy projection.
      double crossXY(const Vector2D& rhs) const
      {
        return VectorUtil::Cross(xy(), rhs);
      }

      /// Calculates the squared length of the vector
      double Mag2() const
      {
        return x() * x() + y() * y() + z() * z();
      }

      /// Calculates the length of the vector
      double R() const
      {
        return hypot3(x(), y(), z());
      }

      /// Scales the vector in place by the given factor
      Vector3D& Scale(const double factor)
      {
        m_xy.Scale(factor);
        m_z *= factor;
        return *this;
      }

      /// Same as Scale()
      Vector3D& operator*=(const double factor)
      {
        return Scale(factor);
      }

      /// Returns a scaled copy of the vector
      Vector3D scaled(const double factor) const
      {
        return Vector3D(xy().scaled(factor), z() * factor);
      }

      /// Same as scaled()
      Vector3D operator*(const double factor) const
      {
        return scaled(factor);
      }

      /// Divides all coordinates by a common denominator in place
      Vector3D& divide(const double denominator)
      {
        m_xy.divide(denominator);
        m_z /= denominator;
        return *this;
      }

      /// Same as divide()
      Vector3D& operator/=(const double denominator)
      {
        return divide(denominator);
      }

      /// Returns a copy where all coordinates got divided by a common denominator
      Vector3D divided(const double denominator) const
      {
        return Vector3D(xy().divided(denominator), z() / denominator);
      }

      /// Same as divided()
      Vector3D operator/(const double denominator) const
      {
        return divided(denominator);
      }

      /// Adds a vector to this in place
      Vector3D& add(const Vector3D& rhs)
      {
        m_xy.add(rhs.xy());
        m_z += rhs.z();
        return *this;
      }

      /// Adds a two dimensional vector to this in place taking z of the given vector as zero
      Vector3D& add(const Vector2D& rhs)
      {
        m_xy.add(rhs);
        return *this;
      }

      /// Same as add()
      Vector3D& operator+=(const Vector3D& rhs)
      {
        return add(rhs);
      }

      /// Same as add()
      Vector3D& operator+=(const Vector2D& rhs)
      {
        return add(rhs);
      }

      /// Subtracts a vector to this in place
      Vector3D& subtract(const Vector3D& rhs)
      {
        m_xy.subtract(rhs.xy());
        m_z -= rhs.z();
        return *this;
      }

      /// Subtracts a two dimensional vector from this in place taking z of the given vector as zero
      Vector3D& subtract(const Vector2D& rhs)
      {
        m_xy.subtract(rhs);
        return *this;
      }

      /// Same as subtract()
      Vector3D& operator-=(const Vector3D& rhs)
      {
        return subtract(rhs);
      }

      /// Same as subtract()
      Vector3D& operator-=(const Vector2D& rhs)
      {
        return subtract(rhs);
      }

      /// Same as reversed()
      Vector3D operator-() const
      {
        return scaled(-1.0);
      }

      /// Returns a new vector as sum of this and rhs
      Vector3D operator+(const Vector3D& rhs) const
      {
        return Vector3D(xy() + rhs.xy(), z() + rhs.z());
      }

      /// Returns a new vector as difference of this and rhs
      Vector3D operator-(const Vector3D& rhs) const
      {
        return Vector3D(xy() - rhs.xy(), z() - rhs.z());
      }

      /// Getter for the x coordinate
      double x() const
      {
        return m_xy.x();
      }

      /// Setter for the x coordinate
      void SetX(const double x)
      {
        m_xy.SetX(x);
      }

      /// Getter for the y coordinate
      double y() const
      {
        return m_xy.y();
      }

      /// Setter for the y coordinate
      void SetY(const double y)
      {
        m_xy.SetY(y);
      }

      /// Getter for the z coordinate
      double z() const
      {
        return m_z;
      }

      /// Setter for the z coordinate
      void SetZ(const double z)
      {
        m_z = z;
      }

      /// Getter for the xy projected vector ( reference ! )
      const Vector2D& xy() const
      {
        return m_xy;
      }

      /// Setter for all three coordinates.
      void set(const double first, const double second, const double third)
      {
        SetX(first);
        SetY(second);
        SetZ(third);
      }

      /// Setter for all three coordinates.
      void SetXYZ(const double x, const double y, const double z)
      {
        SetX(x);
        SetY(y);
        SetZ(z);
      }

      /// Getter for the cylindrical radius ( xy projected norm )
      double Rho() const
      {
        return xy().R();
      }

      /// Getter for the azimuth angle
      double phi() const
      {
        return xy().phi();
      }

      /// Getter for the polar angle
      double theta() const
      {
        return atan2(Rho(), z());
      }

      /// Getter for lambda
      double lambda() const
      {
        return atan2(z(), Rho());
      }

      /// Getter for the cotangent of the polar angle
      double cotTheta() const
      {
        return z() / Rho();
      }

      /// Getter for the tangent of lambda equivalent to cotTheta()
      double tanLambda() const
      {
        return z() / Rho();
      }

    private:
      /// Memory for the first and second coordinate available as a vector
      Vector2D m_xy;

      /// Memory for the third coordinate
      double m_z;
    };

    /// non-memberfunction for subtracting a Vector2D from any generic Vector that implements .X(), .Y(), and Z()
    template<class Vector>
    Vector3D operator- (const Vector& a, const Vector3D& b)
    {
      return Vector3D(a.X() - b.x(), a.Y() - b.y(), a.Z() - b.z());
    }

    /// Output operator for debugging
    std::ostream& operator<<(std::ostream& output, const Vector3D& vector3D);
  }
}
