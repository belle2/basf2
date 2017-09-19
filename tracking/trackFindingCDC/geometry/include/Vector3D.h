/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/numerics/Quadratic.h>

#include <string>
#include <iosfwd>
#include <cmath>

class TVector3;

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A three dimensional vector
    /** A three dimensional vector which is equiped with the expected vector methods. \n
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

      /// Constructs the average of two vectors
      /** Computes the average of two vectors.
       *  If one vector contains NAN the average is the other vector, since the former is not
       *considered a valid value.
       **/
      static Vector3D average(const Vector3D& one, const Vector3D& two)
      {
        if (one.hasNAN()) {
          return two;
        } else if (two.hasNAN()) {
          return one;
        } else {
          return Vector3D((one.x() + two.x()) / 2.0,
                          (one.y() + two.y()) / 2.0,
                          (one.z() + two.z()) / 2.0);
        }
      }

      /// Constructs the average of three vectors
      /** Computes the average of three vectors. In case one of the three dimensional vectors
       *contains an NAN,
       *  it is not considered a valid value for the average and is therefore left out.
       *  The average() of the other two vectors is then returned.
       **/
      static Vector3D average(const Vector3D& one, const Vector3D& two, const Vector3D& three)
      {

        if (one.hasNAN()) {
          return average(two, three);
        } else if (two.hasNAN()) {
          return average(one, three);
        } else if (three.hasNAN()) {
          return average(one, two);
        } else {
          return Vector3D((one.x() + two.x() + three.x()) / 3.0,
                          (one.y() + two.y() + three.y()) / 3.0,
                          (one.z() + two.z() + three.z()) / 3.0);
        }
      }

      /// Casting the back to TVector3 seamlessly
      operator const TVector3() const;

      /// Equality comparison with all three coordinates
      bool operator==(const Vector3D& rhs) const
      {
        return x() == rhs.x() and y() == rhs.y() and z() == rhs.z();
      }

      /// Total ordering based on cylindrical radius first the z component second and azimuth angle
      /// third.
      /** Total order achiving a lower bound Vector3D(0.0, 0.0, 0.0). By first taking the norm \n
       *  for comparision the null vector is smaller than all other possible \n
       *  vectors. Secondly the polar angle theta ( equivalently z ) and finally the azimuth \n
       *  angle phi is considered to have a total ordering for all vectors. \n
       *  Note does not commute with the projection to xy space.
       **/
      bool operator<(const Vector3D& rhs) const
      {
        return norm() < rhs.norm() or (norm() == rhs.norm() and
                                       (z() < rhs.z() or (z() == rhs.z() and (phi() < rhs.phi()))));
      }

      /// Getter for the lowest possible vector
      /** The lowest possilbe vector according to the comparision is the null vector */
      static Vector3D getLowest()
      {
        return Vector3D(0.0, 0.0, 0.0);
      }

      /// Checks if the vector is the null vector.
      bool isNull() const
      {
        return x() == 0.0 and y() == 0.0 and z() == 0.0;
      }

      /// Checks if one of the coordinates is NAN
      bool hasNAN() const
      {
        return std::isnan(x()) or std::isnan(y()) or std::isnan(z());
      }

      /// Output operator for python
      std::string __str__() const;

      /// Calculates the three dimensional dot product.
      double dot(const Vector3D& rhs) const
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
        return xy().cross(rhs.xy());
      }

      /// Calculates the two dimensional cross product in xy projection.
      double crossXY(const Vector2D& rhs) const
      {
        return xy().cross(rhs);
      }

      /// Calculates the squared length of the vector
      double normSquared() const
      {
        return x() * x() + y() * y() + z() * z();
      }

      /// Calculates the length of the vector
      double norm() const
      {
        return hypot3(x(), y(), z());
      }

      /** @name Angle functions
       *  These functions measure the angle between two vectors from *this* to rhs. \n
       *  Because the three dimensional space does not have a unique orientation available, \n
       *  the sign of the angle is not well defined. Therefore angles between three dimensional \n
       *  vectors are always positiv and so is the sine of the angle. The angle is bound by M_PI.
       */
      ///@{
      /// Cosine of the angle between this and rhs
      double cosWith(const Vector3D& rhs) const
      {
        return dot(rhs) / (norm() * rhs.norm());
      }

      /// Sine of the angle between this and rhs
      double sinWith(const Vector3D& rhs) const
      {
        return cross(rhs).norm() / (norm() * rhs.norm());
      }

      /// The angle between this and rhs
      double angleWith(const Vector3D& rhs) const
      {
        return atan2(sinWith(rhs), cosWith(rhs));
      }
      ///@}

      /// Calculates the distance of this point to the rhs
      double distance(const Vector3D& rhs = Vector3D(0.0, 0.0, 0.0)) const
      {
        double deltaX = x() - rhs.x();
        double deltaY = y() - rhs.y();
        double deltaZ = z() - rhs.z();
        return hypot3(deltaX, deltaY, deltaZ);
      }

      /// Scales the vector in place by the given factor
      Vector3D& scale(const double factor)
      {
        m_xy.scale(factor);
        m_z *= factor;
        return *this;
      }

      /// Same as scale()
      Vector3D& operator*=(const double factor)
      {
        return scale(factor);
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

      /// Returns a unit vector colaligned with this
      Vector3D unit() const
      {
        return isNull() ? Vector3D(0.0, 0.0, 0.0) : divided(norm());
      }

      /// Normalizes the vector to unit length
      /** Normalizes the vector to unit length and returns the length the vector had before. \n
       *  The null vector is not transformed. */
      double normalize()
      {
        double result = norm();
        if (result != 0.0) divide(result);
        return result;
      }

      /// Normalizes the vector to the given length
      /** Normalizes the vector to the given length and returns the length the vector had before. \n
       *  The null vector is not transformed. */
      double normalizeTo(const double toLength)
      {
        double result = norm();
        if (result != 0.0) scale(toLength / result);
        return result;
      }

      /// Reverses the direction of the vector in place
      Vector3D& reverse()
      {
        scale(-1.0);
        return *this;
      }

      /// Returns a vector pointing in the opposite direction
      Vector3D reversed() const
      {
        return scaled(-1.0);
      }

      /// Same as reversed()
      Vector3D operator-() const
      {
        return reversed();
      }

      /// Returns a new vector as sum of this and rhs
      Vector3D operator+(const Vector3D& rhs) const
      {
        return Vector3D(xy() + rhs.xy(), z() + rhs.z());
      }

      /// Returns a new vector as differenc of this and rhs
      Vector3D operator-(const Vector3D& rhs) const
      {
        return Vector3D(xy() - rhs.xy(), z() - rhs.z());
      }

      /// Calculates the component parallel to the given vector
      double parallelComp(const Vector3D& relativTo) const
      {
        return relativTo.dot(*this) / relativTo.norm();
      }

      /// Calculates the part of this vector that is parallel to the given vector
      Vector3D parallelVector(const Vector3D& relativTo) const
      {
        return relativTo.scaled(relativTo.dot(*this) / relativTo.normSquared());
      }

      /// Same as parallelComp() but assumes the given vector to be of unit length.
      /** This assumes the given vector relativeTo to be of unit length and avoids \n
       *  a costly computation of the vector norm()*/
      double unnormalizedParallelComp(const Vector3D& relativTo) const
      {
        return relativTo.dot(*this);
      }

      /// Calculates the component orthogonal to the given vector
      /** The orthogonal component is the rest of the vector not parallel to \n
       *  relative to. Since the three dimensional space does not have a unque \n
       *  orientation given by the vector relative to, the sign of the orthogonal \n
       *  component is meaningless and is always set to positiv */
      double orthogonalComp(const Vector3D& relativTo) const
      {
        return relativTo.cross(*this).norm() / relativTo.norm();
      }

      /// Calculates the part of this vector that is parallel to the given vector
      Vector3D orthogonalVector(const Vector3D& relativTo) const
      {
        return *this - parallelVector(relativTo);
      }

      /// Same as orthogonalComp() but assumes the given vector to be of unit length
      /** This assumes the given vector relativeTo to be of unit length and avoids \n
       *  a costly computation of the vector norm()*/
      double unnormalizedOrthogonalComp(const Vector3D& relativTo) const
      {
        return relativTo.cross(*this).norm();
      }

      /// Passivelly moves the vector inplace by the given vector
      void passiveMoveBy(const Vector3D& by)
      {
        subtract(by);
      }

      /// Passivelly moves the vector inplace by the given vector
      Vector3D passiveMovedBy(const Vector3D& by)
      {
        return *this - by;
      }

      /// Getter for the x coordinate
      double x() const
      {
        return m_xy.x();
      }

      /// Setter for the x coordinate
      void setX(const double x)
      {
        m_xy.setX(x);
      }

      /// Getter for the y coordinate
      double y() const
      {
        return m_xy.y();
      }

      /// Setter for the y coordinate
      void setY(const double y)
      {
        m_xy.setY(y);
      }

      /// Getter for the z coordinate
      double z() const
      {
        return m_z;
      }

      /// Setter for the z coordinate
      void setZ(const double z)
      {
        m_z = z;
      }

      /// Getter for the xy projected vector ( reference ! )
      const Vector2D& xy() const
      {
        return m_xy;
      }

      /// Setter for the xy projected vector
      void setXY(const Vector2D& xy)
      {
        m_xy = xy;
      }

      /// Setter for all three coordinates.
      void set(const double first, const double second, const double third)
      {
        setX(first);
        setY(second);
        setZ(third);
      }

      /// Getter for the squared cylindrical radius ( xy projected squared norm )
      double cylindricalRSquared() const
      {
        return xy().normSquared();
      }

      /// Getter for the cylindrical radius ( xy projected norm )
      double cylindricalR() const
      {
        return xy().norm();
      }

      /// Getter for the azimuth angle
      double phi() const
      {
        return xy().phi();
      }

      /// Getter for the polar angle
      double theta() const
      {
        return atan2(cylindricalR(), z());
      }

      /// Getter for lambda
      double lambda() const
      {
        return atan2(z(), cylindricalR());
      }

      /// Getter for the cotangent of the polar angle
      double cotTheta() const
      {
        return z() / cylindricalR();
      }

      /// Getter for the tangent of lambda equivalent to cotTheta()
      double tanLambda() const
      {
        return z() / cylindricalR();
      }

    private:
      /// Memory for the first and second coordinate available as a vector
      Vector2D m_xy;

      /// Memory for the third coordinate
      double m_z;
    };

    /// Output operator for debugging
    std::ostream& operator<<(std::ostream& output, const Vector3D& vector3D);
  }
}
