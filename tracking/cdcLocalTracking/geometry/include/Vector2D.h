/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef VECTOR2D_H
#define VECTOR2D_H

#include <TVector2.h>
#include <math.h>
#include <iostream>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/numerics/numerics.h>
#include <tracking/cdcLocalTracking/typedefs/InfoTypes.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    ///A two dimensional vector
    /** A two dimensional vector which is equiped with functions for correct handeling \n
     *  of orientation related issues in addition to the expected vector methods. \n
     *  It inherits from UsedTObject which can be inject the inheritance to TObject \n
     *  for monitoring purposes but may be empty for maximal performance. \n
     *  Also this vector can be used whereever a TVector2 is expected syntactically.
     */
    class Vector2D : public CDCLocalTracking::UsedTObject {

    public:

      /// Default constructor for ROOT compatibility.
      Vector2D(): m_x(0.0), m_y(0.0) {;}

      /// Constructor translating from a TVector2 instance
      Vector2D(const TVector2& tvector) : m_x(tvector.X()), m_y(tvector.Y()) { ; }

      /// Constructor from two coordinates
      Vector2D(const FloatType& x, const FloatType& y)  : m_x(x), m_y(y) { ; }

      /// Constructs a vector from a unit coordinate system vector and the coordinates in that system. Same as compose()
      /** See compose() for details */
      Vector2D(const Vector2D& coordinateVec,
               const FloatType& parallelCoor,
               const FloatType& orthoCoor) :
        m_x(coordinateVec.x() * parallelCoor - coordinateVec.y() * orthoCoor),
        m_y(coordinateVec.y() * parallelCoor + coordinateVec.x() * orthoCoor) {;}

      /// Constucts a unit vector with polar angle equal to phi
      static inline Vector2D Phi(const FloatType& phi)
      { return isNAN(phi) ? Vector2D(0.0, 0.0) : Vector2D(cos(phi), sin(phi)); }

      /// Constructs a vector from a unit coordinate system vector and the coordinates in that system
      /** Combines a coordinate system vector expressed in laboratory coordinates \n
       *  with the parallel and orthogonal components in the coordinate system \n
       *  to a vector in laboratory coordinates. The coordinate system vector \n
       *  is assumed the unit of the coordinate system */
      static inline Vector2D compose(const Vector2D& coordinateVec,
                                     const FloatType& parallelCoor,
                                     const FloatType& orthoCoor)
      { return Vector2D(coordinateVec , parallelCoor , orthoCoor); }


      /// Constructs the average of two vectors
      /** Computes the average of two vectors
       *  If one vector contains NAN the average is the other vector, since the former is not considered a valid value.
       **/
      static inline Vector2D average(const Vector2D& one , const Vector2D& two) {
        if (one.hasNAN()) {
          return two;
        } else if (two.hasNAN()) {
          return one;
        } else {
          return Vector2D((one.x() + two.x()) / 2.0 , (one.y() + two.y()) / 2.0);
        }
      }


      /// Constructs the average of three vectors
      /** Computes the average of three vectors. In case one of the two dimensional vectors contains an NAN,
       *  it is not considered a valid value for the average and is therefore left out.
       *  The average() of the other two vectors is then returned.
       **/
      static inline Vector2D average(const Vector2D& one , const Vector2D& two , const Vector2D& three) {

        if (one.hasNAN()) {
          return average(two, three);
        } else if (two.hasNAN()) {
          return average(one, three);
        } else if (three.hasNAN()) {
          return average(one, two);
        } else {
          return Vector2D((one.x() + two.x() + three.x()) / 3.0 , (one.y() + two.y() + three.y()) / 3.0);
        }
      }

      /// Empty Destructor.
      ~Vector2D() {;}

      /// Casting the back to TVector2 seamlessly
      inline operator const TVector2() { return TVector2(x(), y()); }

      /// Equality comparison with both coordinates
      inline bool operator==(const Vector2D& rhs) const
      { return x() == rhs.x() and y() == rhs.y(); }

      /// Total ordering based on polar radius first and polar angle second
      /** Total order achiving a lower bound. By first taking the polar radius \n
       *  for comparision the null vector is smaller than all other possible \n
       *  vectors. Secondly the polar angle is considered to have a total ordering \n
       *  for all vectors. The polar angle however is quite costly to compute.\n
       *  This can be optimized if necessary to a different form */
      inline bool operator<(const Vector2D& rhs) const {
        return normSquared() < rhs.normSquared() or (
                 normSquared() == rhs.normSquared() and (
                   phi() < rhs.phi())) ;
      }


      /// Getter for the lowest possible vector
      /** The lowest possilbe vector according to the comparision is the null vector */
      inline static Vector2D getLowest() { return Vector2D(0.0, 0.0); }

      /// Checks if the vector is the null vector.
      inline bool isNull() const { return x() == 0.0 and y() == 0.0; }

      /// Checks if one of the coordinates is NAN
      inline bool hasNAN() const { return isNAN(x()) or isNAN(y()); }

      /// Output operator for debugging
      friend std::ostream& operator<<(std::ostream& output, const Vector2D& vector)
      { output << "Vector2D(" << vector.x() << "," << vector.y() << ")"; return output; }

      /// Calculates the two dimensional dot product.
      inline FloatType dot(const Vector2D& rhs) const { return x() * rhs.x() + y() * rhs.y(); }
      /// Calculated the two dimensional cross product.
      inline FloatType cross(const Vector2D& rhs)const { return x() * rhs.y() - y() * rhs.x(); }

      /// Calculates \f$ x^2 + y^2 \f$ .
      inline FloatType normSquared() const { return x() * x() + y() * y(); }

      /// Calculates the length of the vector.
      inline FloatType norm() const { return hypot(x(), y()); }

      /** @name Angle functions
       *  These functions measure the angle between two vectors from *this* to rhs
       *  in the mathematical positve counterclockwise direction. So a positiv angle means
       *  rhs is more counterclockwise than this.
       */
      ///@{
      ///Cosine of the angle between this and rhs
      inline FloatType cosWith(const Vector2D& rhs) const { return dot(rhs) / (norm() * rhs.norm());  }
      ///Sine of the angle between this and rhs
      inline FloatType sinWith(const Vector2D& rhs) const { return cross(rhs) / (norm() * rhs.norm());  }
      ///The angle between this and rhs
      inline FloatType angleWith(const Vector2D& rhs) const { return atan2(sinWith(rhs), cosWith(rhs)); }
      ///@}

      /// Calculates the distance of this point to the rhs
      inline FloatType distance(const Vector2D& rhs = Vector2D(0.0, 0.0)) const {
        FloatType deltaX = x() - rhs.x();
        FloatType deltaY = y() - rhs.y();
        return hypot(deltaX, deltaY);
      }

      /// Scales the vector in place by the given factor
      inline Vector2D& scale(const FloatType& factor) { m_x *= factor; m_y *= factor; return *this; }
      /// Same as scale()
      inline Vector2D& operator*=(const FloatType& factor) { return scale(factor); }

      /// Returns a scaled copy of the vector
      inline Vector2D scaled(const FloatType& factor) const { return Vector2D(x() * factor, y() * factor); }

      /// Same as scaled()
      inline friend Vector2D operator*(const Vector2D& vec2D, const FloatType& factor)
      { return vec2D.scaled(factor); }

      /// Divides all coordinates by a common denominator in place
      inline Vector2D& divide(const FloatType& denominator) {  m_x /= denominator; m_y /= denominator; return *this; }
      /// Same as divide()
      inline Vector2D& operator/=(const FloatType& denominator) { return divide(denominator); }

      /// Returns a copy where all coordinates got divided by a common denominator
      inline Vector2D divided(const FloatType& denominator) const { return Vector2D(x() / denominator, y() / denominator); }
      /// Same as divided()
      inline Vector2D operator/(const FloatType& denominator) const { return divided(denominator); }

      /// Adds a vector to this in place
      inline Vector2D& add(const Vector2D& rhs)     { m_x += rhs.x();  m_y += rhs.y(); return *this; }
      /// Same as add()
      inline Vector2D& operator+=(const Vector2D& rhs) { return add(rhs); }

      /// Subtracts a vector from this in place
      inline Vector2D& subtract(const Vector2D& rhs) { m_x -= rhs.x();  m_y -= rhs.y(); return *this; }
      /// Same as subtract()
      inline Vector2D& operator-=(const Vector2D& rhs) { return subtract(rhs); }

      /// Orthogonal vector to the counterclockwise direction
      inline Vector2D orthogonal() const { return Vector2D(-y(), x()); }

      /// Orthogonal vector to the direction given by the counterclockwise info
      inline Vector2D orthogonal(const CCWInfo& ccw_info) const { return Vector2D(-ccw_info * y(), ccw_info * x()); }

      /// Normalizes the vector to unit length
      /** Normalizes the vector to unit length and returns the length the vector had before. \n
       *  The null vector is not transformed. */
      inline FloatType normalize()
      { FloatType originalLength = norm(); if (originalLength != 0.0) divide(originalLength); return originalLength; }

      /// Normalizes the vector to the given length
      /** Normalizes the vector to the given length and returns the length the vector had before. \n
       *  The null vector is not transformed. */
      inline FloatType normalizeTo(const FloatType& toLength)
      { FloatType originalLength = norm(); if (originalLength != 0.0) scale(toLength / originalLength); return originalLength; }

      /// Returns a unit vector colaligned with this
      inline Vector2D unit() const
      { return isNull() ? Vector2D(0.0, 0.0) : divided(norm()); }

      /// Reverses the direction of the vector in place
      inline Vector2D& reverse() { scale(-1.0); return *this; }

      /// Returns a vector pointing in the opposite direction
      inline Vector2D reversed() const { return scaled(-1.0); }
      /// Same as reversed()
      inline Vector2D operator-() const { return reversed(); }

      /// Flips the first coordinate inplace (no difference between active and passive)
      void flipFirst()
      { m_x = -x(); }

      /// Flips the first coordinate inplace (no difference between active and passive)
      void flipSecond()
      { m_y = -y(); }

      /// Makes a copy of the vector with the first coordinate flipped (no difference between active and passive)
      Vector2D flippedFirst() const
      { return Vector2D(-x(), y()); }

      /// Makes a copy of the vector with the second coordinate flipped (no difference between active and passive)
      Vector2D flippedSecond() const
      { return Vector2D(x(), -y()); }


      /// Transforms the vector to conformal space inplace
      /** Applies the conformal map in the self-inverse from  X = x / (x^2 + y^2) and Y = y / (x^2 +y^2) inplace */
      inline void conformalTransform() { divide(normSquared()); }

      /// Returns a copy of the vector transformed in conformal space
      /** Applies the conformal map in the self-inverse from  X = x / (x^2 + y^2) and Y = y / (x^2 +y^2) and returns the result as a new Vector2D */
      inline Vector2D conformalTransformed() const { return divided(normSquared()); }

      /// Returns a new vector as sum of this and rhs
      inline Vector2D operator+(const Vector2D& rhs) const
      { return Vector2D(x() + rhs.x(), y() + rhs.y()); }

      /// Returns a new vector as differenc of this and rhs
      inline Vector2D operator-(const Vector2D& rhs) const
      { return Vector2D(x() - rhs.x(), y() - rhs.y()); }

      /// Calculates the component parallel to the given vector
      inline FloatType parallelComp(const Vector2D& relativTo) const
      { return relativTo.dot(*this) / relativTo.norm(); }

      /// Same as parallelComp() but assumes the given vector to be of unit length.
      /** This assumes the given vector relativeTo to be of unit length and avoids \n
       *  a costly computation of the vector norm()*/
      inline FloatType fastParallelComp(const Vector2D& relativTo) const
      { return relativTo.dot(*this); }

      /// Calculates the component orthogonal to the given vector
      /** The orthogonal component is the component parallel to relativeTo.orthogonal() */
      inline FloatType orthogonalComp(const Vector2D& relativTo) const
      { return relativTo.cross(*this) / relativTo.norm(); }

      /// Same as orthogonalComp() but assumes the given vector to be of unit length
      /** This assumes the given vector relativeTo to be of unit length and avoids \n
       *  a costly computation of the vector norm()*/
      inline FloatType fastOrthogonalComp(const Vector2D& relativTo) const
      { return relativTo.cross(*this); }

      /// Indicates if the given vector is more left or more right if you looked in the direction of this vector.
      inline RightLeftInfo isRightOrLeftOf(const Vector2D& rhs) const
      { return -CDCLocalTracking::sign(fastOrthogonalComp(rhs)); }

      /// Indicates if the given vector is more left if you looked in the direction of this vector.
      inline bool isLeftOf(const Vector2D& rhs) const
      { return isRightOrLeftOf(rhs) == LEFT; }

      /// Indicates if the given vector is more right if you looked in the direction of this vector.
      inline bool isRightOf(const Vector2D& rhs) const
      { return isRightOrLeftOf(rhs) == RIGHT; }


      /// Indicates if the given vector is more counterclockwise or more clockwise if you looked in the direction of this vector.
      inline CCWInfo isCCWOrCWOf(const Vector2D& rhs) const
      { return CDCLocalTracking::sign(fastOrthogonalComp(rhs)); }

      /// Indicates if the given vector is more counterclockwise if you looked in the direction of this vector.
      inline bool isCCWOf(const Vector2D& rhs) const
      { return isCCWOrCWOf(rhs) == CCW; }

      /// Indicates if the given vector is more clockwise if you looked in the direction of this vector.
      inline bool isCWOf(const Vector2D& rhs) const
      { return isCCWOrCWOf(rhs) == CW; }


      /// Indicates if the given vector is more coaligned or reverse if you looked in the direction of this vector.
      inline ForwardBackwardInfo isForwardOrBackwardOf(const Vector2D& rhs) const
      { return CDCLocalTracking::sign(fastParallelComp(rhs)); }

      /// Indicates if the given vector is more coaligned if you looked in the direction of this vector.
      inline bool isForwardOf(const Vector2D& rhs) const
      { return isForwardOrBackwardOf(rhs) == FORWARD; }

      /// Indicates if the given vector is more Reverse if you looked in the direction of this vector.
      inline bool isBackwardOf(const Vector2D& rhs) const
      { return isForwardOrBackwardOf(rhs) == BACKWARD; }



      /// Swaps the coordinates in place
      inline void swapCoordinates() { std::swap(m_x, m_y); }

      /// Gives the polar radius of the vector. Same as norm()
      inline FloatType polarR() const { return hypot(x(), y()); }

      /// Set the polar radius while keeping the polar angle phi the same
      inline void setPolarR(const FloatType& polar_r)
      { scale(polar_r / norm()); }

      /// Gives the polar radius being the angle to the x axes ( range -PI to PI )
      inline FloatType phi() const { return isNull() ? NAN : atan2(y(), x()) ; }

      /// Getter for the x coordinate
      inline const FloatType& x() const { return m_x; }
      /// Setter for the x coordinate
      inline void setX(const FloatType& x) { m_x = x; }
      /// Getter for the y coordinate
      inline const FloatType& y() const { return m_y; }
      /// Setter for the y coordinate
      inline void setY(const FloatType& y) { m_y = y; }

      /// Setter for both coordinate
      inline void setXY(const FloatType& x, const FloatType& y) { setX(x); setY(y); }
      /// Setter for both coordinate by an other vector
      inline void setXY(const Vector2D& xy) { m_x = xy.x(); m_y = xy.y(); }

      /// Getter for the first coordinate
      inline const FloatType& first() const { return m_x; }
      /// Setter for the first coordinate
      inline void setFirst(const FloatType& first) { m_x = first; }
      /// Getter for the second coordinate
      inline const FloatType& second() const { return m_y; }
      /// Setter for the second coordinate
      inline void setSecond(const FloatType& second) { m_y = second; }

      /// Setter for both  coordinate
      inline void set(const FloatType& first, const FloatType& second) { setX(first); setY(second); }
      /// Setter for both coordinate by an other vector
      inline void set(const Vector2D& both) { m_x = both.x(); m_y = both.y(); }

    private:
      FloatType m_x; ///< Memory for the first coordinate
      FloatType m_y; ///< Memory for the second coordinate

    private:
      /// ROOT Macro to make Vector2D a ROOT class.
      ClassDefInCDCLocalTracking(Vector2D, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // VECTOR2D_WITHROOT_H
