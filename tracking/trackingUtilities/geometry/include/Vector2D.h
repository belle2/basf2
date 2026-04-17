/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/numerics/Quadratic.h>

#include <tracking/trackingUtilities/numerics/EForwardBackward.h>
#include <tracking/trackingUtilities/numerics/ERightLeft.h>
#include <tracking/trackingUtilities/numerics/ERotation.h>
#include <tracking/trackingUtilities/numerics/ESign.h>

#include <framework/geometry/VectorUtil.h>

#include <Math/Vector2D.h>

#include <utility>
#include <string>
#include <iosfwd>
#include <cmath>

namespace Belle2 {
  namespace TrackingUtilities {
    /**
     * A two dimensional vector which is equipped with functions for correct handling \n
     * of orientation related issues in addition to the expected vector methods. \n
     */
    class Vector2D {

    public:
      /// Default constructor for ROOT compatibility.
      Vector2D()
        : m_x(0.0)
        , m_y(0.0)
      {
      }

      // /// Constructor translating from a ROOT::Math::XYVector instance
      // TODO / FIXME:
      //   had to comment this explicit constructor because of compilation issues that I did not
      //   fully understand nor really wanted to investigate and solve - just using the non-explicit
      //   version solved all the issues, but for the future I think it would be better to solve this
      //   issue and use the explicit constructor
      // explicit Vector2D(const ROOT::Math::XYVector& xyVector)
      /// Constructor translating from a ROOT::Math::XYVector instance
      Vector2D(const ROOT::Math::XYVector& xyVector);

      /// Constructor from two coordinates
      Vector2D(const double x, const double y)
        : m_x(x)
        , m_y(y)
      {
      }

      /// Assignment translating from a ROOT::Math::XYVector instance
      Vector2D& operator=(const ROOT::Math::XYVector& xyVector);

      /// Casting the back to ROOT::Math::XYVector seamlessly
      operator const ROOT::Math::XYVector() const;

      /// Constructs a unit vector with azimuth angle equal to phi
      static Vector2D Phi(const double phi)
      {
        return std::isnan(phi) ? Vector2D(0.0, 0.0) : Vector2D(cos(phi), sin(phi));
      }

      /// Equality comparison with both coordinates
      bool operator==(const Vector2D& rhs) const
      {
        return x() == rhs.x() and y() == rhs.y();
      }

      /// Total ordering based on cylindrical radius first and azimuth angle second
      /** Total order achieving a absolute lower bound Vector2D(0.0, 0.0). By first taking the
       * cylindrical radius \n
       *  for comparison the null vector is smaller than all other possible \n
       *  vectors. Secondly the azimuth angle is considered to have a total ordering \n
       *  for all vectors.\n
       */
      bool operator<(const Vector2D& rhs) const
      {
        return normSquared() < rhs.normSquared() or
               (normSquared() == rhs.normSquared() and (phi() < rhs.phi()));
      }

      /// Calculates the two dimensional dot product.
      double dot(const Vector2D& rhs) const
      {
        return x() * rhs.x() + y() * rhs.y();
      }
      /// Calculates the two dimensional dot product.
      double Dot(const Vector2D& rhs) const
      {
        return dot(rhs);
      }

      /// Calculated the two dimensional cross product.
      double cross(const Vector2D& rhs) const
      {
        return x() * rhs.y() - y() * rhs.x();
      }

      /// Calculates \f$ x^2 + y^2 \f$ .
      double normSquared() const
      {
        return x() * x() + y() * y();
      }
      /// Alias for normSquared
      double Mag2() const
      {
        return normSquared();
      }

      /// Calculates the length of the vector.
      double R() const
      {
        return hypot2(x(), y());
      }

      /// Calculates the distance of this point to the rhs
      double distance(const Vector2D& rhs = Vector2D(0.0, 0.0)) const
      {
        double deltaX = x() - rhs.x();
        double deltaY = y() - rhs.y();
        return hypot2(deltaX, deltaY);
      }

      /// Scales the vector in place by the given factor
      Vector2D& scale(const double factor)
      {
        m_x *= factor;
        m_y *= factor;
        return *this;
      }
      /// Same as scale()
      Vector2D& operator*=(const double factor)
      {
        return scale(factor);
      }

      /// Returns a scaled copy of the vector
      Vector2D scaled(const double factor) const
      {
        return Vector2D(x() * factor, y() * factor);
      }

      /// Same as scaled()
      friend Vector2D operator*(const Vector2D& vec2D, const double factor)
      {
        return vec2D.scaled(factor);
      }

      /// Divides all coordinates by a common denominator in place
      Vector2D& divide(const double denominator)
      {
        m_x /= denominator;
        m_y /= denominator;
        return *this;
      }

      /// Same as divide()
      Vector2D& operator/=(const double denominator)
      {
        return divide(denominator);
      }

      /// Returns a copy where all coordinates got divided by a common denominator
      Vector2D divided(const double denominator) const
      {
        return Vector2D(x() / denominator, y() / denominator);
      }
      /// Same as divided()
      Vector2D operator/(const double denominator) const
      {
        return divided(denominator);
      }

      /// Adds a vector to this in place
      Vector2D& add(const Vector2D& rhs)
      {
        m_x += rhs.x();
        m_y += rhs.y();
        return *this;
      }

      /// Same as add()
      Vector2D& operator+=(const Vector2D& rhs)
      {
        return add(rhs);
      }

      /// Subtracts a vector from this in place
      Vector2D& subtract(const Vector2D& rhs)
      {
        m_x -= rhs.x();
        m_y -= rhs.y();
        return *this;
      }
      /// Same as subtract()
      Vector2D& operator-=(const Vector2D& rhs)
      {
        return subtract(rhs);
      }

      /// Orthogonal vector to the counterclockwise direction
      Vector2D orthogonal() const
      {
        return Vector2D(-y(), x());
      }

      /// Orthogonal vector to the direction given by the counterclockwise info
      Vector2D orthogonal(const ERotation ccwInfo) const
      {
        return isValid(ccwInfo) ? Vector2D(-static_cast<double>(ccwInfo) * y(), static_cast<double>(ccwInfo) * x()) : Vector2D();
      }

      /// Normalizes the vector to unit length
      /** Normalizes the vector to unit length and returns the length the vector had before. \n
       *  The null vector is not transformed. */
      double normalize()
      {
        double originalLength = R();
        if (originalLength != 0.0) divide(originalLength);
        return originalLength;
      }

      /// Normalizes the vector to the given length
      /** Normalizes the vector to the given length and returns the length the vector had before. \n
       *  The null vector is not transformed. */
      double normalizeTo(const double toLength)
      {
        double originalLength = R();
        if (originalLength != 0.0) scale(toLength / originalLength);
        return originalLength;
      }

      /// Returns a unit vector colaligned with this
      Vector2D unit() const
      {
        return (x() == 0.0 and y() == 0.0) ? Vector2D(0.0, 0.0) : divided(R());
      }

      /// Reverses the direction of the vector in place
      Vector2D& reverse()
      {
        scale(-1.0);
        return *this;
      }

      /// Returns a vector pointing in the opposite direction
      Vector2D reversed() const
      {
        return scaled(-1.0);
      }
      /// Same as reversed()
      Vector2D operator-() const
      {
        return reversed();
      }

      /// Reflects this vector over line designated by the given vector.
      Vector2D flippedOver(const Vector2D& reflectionLine) const
      {
        return *this - orthogonalVector(reflectionLine) * 2;
      }

      /// Reflects this vector along line designated by the given vector.
      Vector2D flippedAlong(const Vector2D& flippingDirection) const
      {
        return *this - Belle2::VectorUtil::parallelVector(*this, flippingDirection) * 2;
      }

      /// Transforms the vector to conformal space inplace
      /** Applies the conformal map in the self-inverse from  X = x / (x^2 + y^2) and Y = y / (x^2
       * +y^2) inplace */
      void conformalTransform()
      {
        divide(normSquared());
      }

      /// Returns a copy of the vector transformed in conformal space
      /** Applies the conformal map in the self-inverse from  X = x / (x^2 + y^2) and Y = y / (x^2
       * +y^2) and returns the result as a new Vector2D */
      Vector2D conformalTransformed() const
      {
        return divided(normSquared());
      }

      /// Returns a new vector as sum of this and rhs
      Vector2D operator+(const Vector2D& rhs) const
      {
        return Vector2D(x() + rhs.x(), y() + rhs.y());
      }

      /// Returns a new vector as differenc of this and rhs
      Vector2D operator-(const Vector2D& rhs) const
      {
        return Vector2D(x() - rhs.x(), y() - rhs.y());
      }

      /// Returns a new vector as differenc of this and rhs
      Vector2D operator-(const ROOT::Math::XYVector& rhs) const
      {
        return Vector2D(x() - rhs.X(), y() - rhs.Y());
      }

      /// Calculates the component parallel to the given vector
      double parallelComp(const Vector2D& relativTo) const
      {
        return relativTo.dot(*this) / relativTo.R();
      }

      /// Same as parallelComp() but assumes the given vector to be of unit length.
      /** This assumes the given vector relativeTo to be of unit length and avoids \n
       *  a costly computation of the vector R()*/
      double unnormalizedParallelComp(const Vector2D& relativTo) const
      {
        return relativTo.dot(*this);
      }

      /// Calculates the component orthogonal to the given vector
      /** The orthogonal component is the component parallel to relativeTo.orthogonal() */
      double orthogonalComp(const Vector2D& relativTo) const
      {
        return relativTo.cross(*this) / relativTo.R();
      }

      /// Calculates the part of this vector that is parallel to the given vector
      Vector2D orthogonalVector(const Vector2D& relativTo) const
      {
        return relativTo.scaled(relativTo.cross(*this) / relativTo.normSquared()).orthogonal();
      }

      /// Same as orthogonalComp() but assumes the given vector to be of unit length
      /** This assumes the given vector relativeTo to be of unit length and avoids \n
       *  a costly computation of the vector R()*/
      double unnormalizedOrthogonalComp(const Vector2D& relativTo) const
      {
        return relativTo.cross(*this);
      }

      /// Indicates if the given vector is more left or more right if you looked in the direction of
      /// this vector.
      ERightLeft isRightOrLeftOf(const Vector2D& rhs) const
      {
        return static_cast<ERightLeft>(-sign(unnormalizedOrthogonalComp(rhs)));
      }

      /// Indicates if the given vector is more left if you looked in the direction of this vector.
      bool isLeftOf(const Vector2D& rhs) const
      {
        return isRightOrLeftOf(rhs) == ERightLeft::c_Left;
      }

      /// Indicates if the given vector is more right if you looked in the direction of this vector.
      bool isRightOf(const Vector2D& rhs) const
      {
        return isRightOrLeftOf(rhs) == ERightLeft::c_Right;
      }

      /// Indicates if the given vector is more counterclockwise or more clockwise if you looked in
      /// the direction of this vector.
      ERotation isCCWOrCWOf(const Vector2D& rhs) const
      {
        return static_cast<ERotation>(sign(unnormalizedOrthogonalComp(rhs)));
      }

      /// Indicates if the given vector is more counterclockwise if you looked in the direction of
      /// this vector.
      bool isCCWOf(const Vector2D& rhs) const
      {
        return isCCWOrCWOf(rhs) == ERotation::c_CounterClockwise;
      }

      /// Indicates if the given vector is more clockwise if you looked in the direction of this
      /// vector.
      bool isCWOf(const Vector2D& rhs) const
      {
        return isCCWOrCWOf(rhs) == ERotation::c_Clockwise;
      }

      /// Indicates if the given vector is more coaligned or reverse if you looked in the direction
      /// of this vector.
      EForwardBackward isForwardOrBackwardOf(const Vector2D& rhs) const
      {
        return static_cast<EForwardBackward>(sign(unnormalizedParallelComp(rhs)));
      }

      /// Indicates if the given vector is more coaligned if you looked in the direction of this
      /// vector.
      bool isForwardOf(const Vector2D& rhs) const
      {
        return isForwardOrBackwardOf(rhs) == EForwardBackward::c_Forward;
      }

      /// Indicates if the given vector is more Reverse if you looked in the direction of this
      /// vector.
      bool isBackwardOf(const Vector2D& rhs) const
      {
        return isForwardOrBackwardOf(rhs) == EForwardBackward::c_Backward;
      }

    private:
      /// Check if three values have the same sign.
      static bool sameSign(float n1, float n2, float n3)
      {
        return ((n1 > 0 and n2 > 0 and n3 > 0) or (n1 < 0 and n2 < 0 and n3 < 0));
      }

    public:
      /** Checks if this vector is between two other vectors
       *  Between means here that when rotating the lower vector (first argument)
       *  mathematically positively it becomes coaligned with this vector before
       *  it becomes coalgined with the other vector.
       */
      bool isBetween(const Vector2D& lower, const Vector2D& upper) const
      {
        // Set up a linear (nonorthogonal) transformation that maps
        // lower -> (1, 0)
        // upper -> (0, 1)
        // Check whether this transformation is orientation conserving
        // If yes this vector must lie in the first quadrant to be between lower and upper
        // If no it must lie in some other quadrant.
        double det = lower.cross(upper);
        if (det == 0) {
          // lower and upper are coaligned
          return isRightOf(lower) and isLeftOf(upper);
        } else {
          bool flipsOrientation = det < 0;

          double transformedX = cross(upper);
          double transformedY = -cross(lower);
          bool inFirstQuadrant = sameSign(det, transformedX, transformedY);
          if (flipsOrientation) {
            inFirstQuadrant = not inFirstQuadrant;
          }
          return inFirstQuadrant;
        }
      }

      /// Swaps the coordinates in place
      void swapCoordinates()
      {
        std::swap(m_x, m_y);
      }

      /// Set the cylindrical radius while keeping the azimuth angle phi the same
      void setCylindricalR(const double cylindricalR)
      {
        scale(cylindricalR / R());
      }

      /// Gives the azimuth angle being the angle to the x axes ( range -M_PI to M_PI )
      double phi() const
      {
        return (x() == 0.0 and y() == 0.0) ? NAN : atan2(y(), x());
      }

      /// Passivelly moves the vector inplace by the given vector
      void passiveMoveBy(const Vector2D& by)
      {
        subtract(by);
      }

      /// Returns a transformed vector passivelly moved by the given vector.
      Vector2D passiveMovedBy(const Vector2D& by) const
      {
        return *this - by;
      }

      /** Returns a transformed vector version rotated  by the given vector.
       *  The rotated coordinates are such that the given phiVec becomes the new x axes.
       *  @param phiVec *Unit* vector marking the x axes of the new rotated coordinate system*/
      Vector2D passiveRotatedBy(const Vector2D& phiVec) const
      {
        return Vector2D(unnormalizedParallelComp(phiVec), unnormalizedOrthogonalComp(phiVec));
      }

      /// Getter for the x coordinate
      double x() const
      {
        return m_x;
      }
      /// Getter for the x coordinate
      double X() const
      {
        return m_x;
      }
      /// Setter for the x coordinate
      void SetX(const double x)
      {
        m_x = x;
      }
      /// Getter for the y coordinate
      double y() const
      {
        return m_y;
      }
      /// Getter for the y coordinate
      double Y() const
      {
        return m_y;
      }
      /// Setter for the y coordinate
      void SetY(const double y)
      {
        m_y = y;
      }

      /// Setter for both coordinate
      void setXY(const double x, const double y)
      {
        SetX(x);
        SetY(y);
      }

      /// Setter for both coordinate
      void SetXY(const double x, const double y)
      {
        SetX(x);
        SetY(y);
      }

      /// Setter for both  coordinate
      void set(const double first, const double second)
      {
        SetX(first);
        SetY(second);
      }
      /// Setter for both coordinate by an other vector
      void set(const Vector2D& both)
      {
        m_x = both.x();
        m_y = both.y();
      }

    private:
      /// Memory for the first coordinate
      double m_x;

      /// Memory for the second coordinate
      double m_y;
    };

    /// non-memberfunction for subtracting a Vector2D from any generic Vector that implements .X() and .Y()
    template<class Vector>
    Vector2D operator- (const Vector& a, const Vector2D& b)
    {
      return Vector2D(a.X() - b.X(), a.Y() - b.Y());
    }

    /// Output operator for debugging
    std::ostream& operator<<(std::ostream& output, const Vector2D& vector2D);
  }
}
