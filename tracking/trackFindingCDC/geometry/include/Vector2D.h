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

#include <tracking/trackFindingCDC/numerics/Quadratic.h>

#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>
#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/numerics/ERotation.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <utility>
#include <string>
#include <iosfwd>
#include <cmath>

class TVector2;

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * A two dimensional vector which is equipped with functions for correct handeling \n
     * of orientation related issues in addition to the expected vector methods. \n
     * Also this vector can be passed to functions where a TVector2 is expected syntactically.
     */
    class Vector2D {

    public:
      /// Default constructor for ROOT compatibility.
      Vector2D()
        : m_x(0.0)
        , m_y(0.0)
      {
      }

      /// Constructor translating from a TVector2 instance
      explicit Vector2D(const TVector2& tVector2);

      /// Constructor from two coordinates
      Vector2D(const double x, const double y)
        : m_x(x)
        , m_y(y)
      {
      }

      /**
       * Constructs a vector from a unit coordinate system vector and the coordinates in that
       * system.
       * Same as compose()
       * See compose() for details.
       */
      Vector2D(const Vector2D& coordinateVec, const double parallelCoor, const double orthoCoor)
        : m_x(coordinateVec.x() * parallelCoor - coordinateVec.y() * orthoCoor)
        , m_y(coordinateVec.y() * parallelCoor + coordinateVec.x() * orthoCoor)
      {
      }

      /// Assignment translating from a TVector2 instance
      Vector2D& operator=(const TVector2& tvector);

      /// Constucts a unit vector with azimuth angle equal to phi
      static Vector2D Phi(const double phi)
      {
        return std::isnan(phi) ? Vector2D(0.0, 0.0) : Vector2D(cos(phi), sin(phi));
      }

      /// Constructs a vector from a unit coordinate system vector and the coordinates in that
      /// system
      /** Combines a coordinate system vector expressed in laboratory coordinates \n
       *  with the parallel and orthogonal components in the coordinate system \n
       *  to a vector in laboratory coordinates. The coordinate system vector \n
       *  is assumed the unit of the coordinate system */
      static Vector2D
      compose(const Vector2D& coordinateVec, const double parallelCoor, const double orthoCoor)
      {
        return Vector2D(coordinateVec, parallelCoor, orthoCoor);
      }

      /// Constructs the average of two vectors
      /** Computes the average of two vectors
       *  If one vector contains NAN the average is the other vector, since the former is not
       *considered a valid value.
       **/
      static Vector2D average(const Vector2D& one, const Vector2D& two)
      {
        if (one.hasNAN()) {
          return two;
        } else if (two.hasNAN()) {
          return one;
        } else {
          return Vector2D((one.x() + two.x()) / 2.0, (one.y() + two.y()) / 2.0);
        }
      }

      /// Constructs the average of three vectors
      /** Computes the average of three vectors. In case one of the two dimensional vectors contains
       *an NAN,
       *  it is not considered a valid value for the average and is therefore left out.
       *  The average() of the other two vectors is then returned.
       **/
      static Vector2D average(const Vector2D& one, const Vector2D& two, const Vector2D& three)
      {

        if (one.hasNAN()) {
          return average(two, three);
        } else if (two.hasNAN()) {
          return average(one, three);
        } else if (three.hasNAN()) {
          return average(one, two);
        } else {
          return Vector2D((one.x() + two.x() + three.x()) / 3.0,
                          (one.y() + two.y() + three.y()) / 3.0);
        }
      }

      /// Casting the back to TVector2 seamlessly
      operator const TVector2();

      /// Equality comparison with both coordinates
      bool operator==(const Vector2D& rhs) const
      {
        return x() == rhs.x() and y() == rhs.y();
      }

      /// Total ordering based on cylindrical radius first and azimuth angle second
      /** Total order achiving a absolute lower bound Vector2D(0.0, 0.0). By first taking the
       * cylindrical radius \n
       *  for comparision the null vector is smaller than all other possible \n
       *  vectors. Secondly the azimuth angle is considered to have a total ordering \n
       *  for all vectors.\n
       */
      bool operator<(const Vector2D& rhs) const
      {
        return normSquared() < rhs.normSquared() or
               (normSquared() == rhs.normSquared() and (phi() < rhs.phi()));
      }

      /// Getter for the lowest possible vector
      /** The lowest possilbe vector according to the comparision is the null vector */
      static Vector2D getLowest()
      {
        return Vector2D(0.0, 0.0);
      }

      /// Checks if the vector is the null vector.
      bool isNull() const
      {
        return x() == 0.0 and y() == 0.0;
      }

      /// Checks if one of the coordinates is NAN
      bool hasNAN() const
      {
        return std::isnan(x()) or std::isnan(y());
      }

      /// Output operator for python
      std::string __str__() const;

      /// Calculates the two dimensional dot product.
      double dot(const Vector2D& rhs) const
      {
        return x() * rhs.x() + y() * rhs.y();
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

      /// Calculates the length of the vector.
      double norm() const
      {
        return hypot2(x(), y());
      }

      /** @name Angle functions
       *  These functions measure the angle between two vectors from *this* to rhs
       *  in the mathematical positve counterclockwise direction. So a positiv angle means
       *  rhs is more counterclockwise than this.
       */
      ///@{
      /// Cosine of the angle between this and rhs
      double cosWith(const Vector2D& rhs) const
      {
        return dot(rhs) / (norm() * rhs.norm());
      }
      /// Sine of the angle between this and rhs
      double sinWith(const Vector2D& rhs) const
      {
        return cross(rhs) / (norm() * rhs.norm());
      }
      /// The angle between this and rhs
      double angleWith(const Vector2D& rhs) const
      {
        return atan2(cross(rhs), dot(rhs));
      }
      ///@}

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
        return isValid(ccwInfo) ? Vector2D(-ccwInfo * y(), ccwInfo * x()) : Vector2D();
      }

      /// Normalizes the vector to unit length
      /** Normalizes the vector to unit length and returns the length the vector had before. \n
       *  The null vector is not transformed. */
      double normalize()
      {
        double originalLength = norm();
        if (originalLength != 0.0) divide(originalLength);
        return originalLength;
      }

      /// Normalizes the vector to the given length
      /** Normalizes the vector to the given length and returns the length the vector had before. \n
       *  The null vector is not transformed. */
      double normalizeTo(const double toLength)
      {
        double originalLength = norm();
        if (originalLength != 0.0) scale(toLength / originalLength);
        return originalLength;
      }

      /// Returns a unit vector colaligned with this
      Vector2D unit() const
      {
        return isNull() ? Vector2D(0.0, 0.0) : divided(norm());
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

      /// Flips the first coordinate inplace (no difference between active and passive)
      void flipFirst()
      {
        m_x = -x();
      }

      /// Flips the first coordinate inplace (no difference between active and passive)
      void flipSecond()
      {
        m_y = -y();
      }

      /// Makes a copy of the vector with the first coordinate flipped (no difference between active
      /// and passive)
      Vector2D flippedFirst() const
      {
        return Vector2D(-x(), y());
      }

      /// Makes a copy of the vector with the second coordinate flipped (no difference between
      /// active and passive)
      Vector2D flippedSecond() const
      {
        return Vector2D(x(), -y());
      }

      /// Reflects this vector over line designated by the given vector.
      Vector2D flippedOver(const Vector2D& reflectionLine) const
      {
        return *this - orthogonalVector(reflectionLine) * 2;
      }

      /// Reflects this vector along line designated by the given vector.
      Vector2D flippedAlong(const Vector2D& flippingDirection) const
      {
        return *this - parallelVector(flippingDirection) * 2;
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

      /// Calculates the component parallel to the given vector
      double parallelComp(const Vector2D& relativTo) const
      {
        return relativTo.dot(*this) / relativTo.norm();
      }

      /// Calculates the part of this vector that is parallel to the given vector
      Vector2D parallelVector(const Vector2D& relativTo) const
      {
        return relativTo.scaled(relativTo.dot(*this) / relativTo.normSquared());
      }

      /// Same as parallelComp() but assumes the given vector to be of unit length.
      /** This assumes the given vector relativeTo to be of unit length and avoids \n
       *  a costly computation of the vector norm()*/
      double unnormalizedParallelComp(const Vector2D& relativTo) const
      {
        return relativTo.dot(*this);
      }

      /// Calculates the component orthogonal to the given vector
      /** The orthogonal component is the component parallel to relativeTo.orthogonal() */
      double orthogonalComp(const Vector2D& relativTo) const
      {
        return relativTo.cross(*this) / relativTo.norm();
      }

      /// Calculates the part of this vector that is parallel to the given vector
      Vector2D orthogonalVector(const Vector2D& relativTo) const
      {
        return relativTo.scaled(relativTo.cross(*this) / relativTo.normSquared()).orthogonal();
      }

      /// Same as orthogonalComp() but assumes the given vector to be of unit length
      /** This assumes the given vector relativeTo to be of unit length and avoids \n
       *  a costly computation of the vector norm()*/
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

      /// Gives the cylindrical radius of the vector. Same as norm()
      double cylindricalR() const
      {
        return hypot2(x(), y());
      }

      /// Set the cylindrical radius while keeping the azimuth angle phi the same
      void setCylindricalR(const double cylindricalR)
      {
        scale(cylindricalR / norm());
      }

      /// Gives the azimuth angle being the angle to the x axes ( range -M_PI to M_PI )
      double phi() const
      {
        return isNull() ? NAN : atan2(y(), x());
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
      /// Setter for the x coordinate
      void setX(const double x)
      {
        m_x = x;
      }
      /// Getter for the y coordinate
      double y() const
      {
        return m_y;
      }
      /// Setter for the y coordinate
      void setY(const double y)
      {
        m_y = y;
      }

      /// Setter for both coordinate
      void setXY(const double x, const double y)
      {
        setX(x);
        setY(y);
      }
      /// Setter for both coordinate by an other vector
      void setXY(const Vector2D& xy)
      {
        m_x = xy.x();
        m_y = xy.y();
      }

      /// Getter for the first coordinate
      double first() const
      {
        return m_x;
      }
      /// Setter for the first coordinate
      void setFirst(const double first)
      {
        m_x = first;
      }
      /// Getter for the second coordinate
      double second() const
      {
        return m_y;
      }
      /// Setter for the second coordinate
      void setSecond(const double second)
      {
        m_y = second;
      }

      /// Setter for both  coordinate
      void set(const double first, const double second)
      {
        setX(first);
        setY(second);
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

    /// Output operator for debugging
    std::ostream& operator<<(std::ostream& output, const Vector2D& vector2D);
  }
}
