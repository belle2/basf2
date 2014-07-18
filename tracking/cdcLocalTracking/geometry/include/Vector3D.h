/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <math.h>
#include <iostream>

#include <TVector3.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/numerics/numerics.h>

#include "Vector2D.h"


namespace Belle2 {
  namespace CDCLocalTracking {

    ///A three dimensional vector
    /** A three dimensional vector which is equiped with the expected vector methods. \n
     *  It inherits from UsedTObject which can be inject the inheritance to TObject \n
     *  for monitoring purposes but may be empty for maximal performance. \n
     *  Also this vector can be used whereever a TVector3 is expected syntactically.
     */
    class Vector3D : public CDCLocalTracking::UsedTObject {

    public:



      /// Default constructor for ROOT compatibility.
      Vector3D(): m_xy(0.0, 0.0), m_z(0.0) {;}

      /// Constructor translating from a TVector3 instance
      Vector3D(TVector3 tvector) : m_xy(tvector.X(), tvector.Y()), m_z(tvector.Z()) { ; }

      /// Constructor from three coordinates
      Vector3D(FloatType x, FloatType y, FloatType z)  : m_xy(x, y), m_z(z) { ; }

      /// Constructor augmeting a Vector2D to a Vector3D setting z to zero
      explicit Vector3D(const Vector2D& xy)  : m_xy(xy), m_z(0.0) { ; }

      /// Constructor augmeting a Vector2D to a Vector3D setting z explicitly
      Vector3D(const Vector2D& xy, FloatType z)  : m_xy(xy), m_z(z) { ; }

      /// Constructs the average of two vectors
      /** Computes the average of two vectors.
       *  If one vector contains NAN the average is the other vector, since the former is not considered a valid value.
       **/
      static inline Vector3D average(const Vector3D& one , const Vector3D& two) {
        if (one.hasNAN()) {
          return two;
        } else if (two.hasNAN()) {
          return one;
        } else {
          return Vector3D((one.x() + two.x()) / 2.0 ,
                          (one.y() + two.y()) / 2.0 ,
                          (one.z() + two.z()) / 2.0);
        }
      }

      /// Constructs the average of three vectors
      /** Computes the average of three vectors. In case one of the three dimensional vectors contains an NAN,
       *  it is not considered a valid value for the average and is therefore left out.
       *  The average() of the other two vectors is then returned.
       **/
      static inline Vector3D average(const Vector3D& one , const Vector3D& two , const Vector3D& three) {

        if (one.hasNAN()) {
          return average(two, three);
        } else if (two.hasNAN()) {
          return average(one, three);
        } else if (three.hasNAN()) {
          return average(one, two);
        } else {
          return Vector3D((one.x() + two.x() + three.x()) / 3.0 ,
                          (one.y() + two.y() + three.y()) / 3.0 ,
                          (one.z() + two.z() + three.z()) / 3.0);
        }

      }

      /// Empty Destructor.
      ~Vector3D() {;}

      /// Casting the back to TVector3 seamlessly
      inline operator const TVector3() { return TVector3(x(), y(), z()); }

      /// Equality comparison with all three coordinates
      inline bool operator==(const Vector3D& rhs) const
      { return x() == rhs.x() and y() == rhs.y() and z() == rhs.z(); }

      /// Total ordering based on polar radius first and polar angle second
      /** Total order achiving a lower bound. By first taking the norm \n
       *  for comparision the null vector is smaller than all other possible \n
       *  vectors. Secondly the theta ( equivalently z ) and finally the polar \n
       *  angle is considered to have a total ordering for all vectors. \n
       *  The polar angle however is quite costly to compute.\n
       *  This can be optimized if necessary to a different form.
       *  Note does not commute with the projection to xy space
       **/
      inline bool operator<(const Vector3D& rhs) const {
        return norm() < rhs.norm() or (
                 norm() == rhs.norm() and (
                   z() < rhs.z() or (
                     z() == rhs.z() and (
                       phi() < rhs.phi()))));
      }

      /// Getter for the lowest possible vector
      /** The lowest possilbe vector according to the comparision is the null vector */
      inline static Vector3D getLowest() { return Vector3D(0.0, 0.0, 0.0); }

      /// Checks if the vector is the null vector.
      inline bool isNull() const { return x() == 0.0 and y() == 0.0 and z() == 0.0 ; }

      /// Checks if one of the coordinates is NAN
      inline bool hasNAN() const { return isNAN(x()) or isNAN(y()) or isNAN(z()); }

      /// Output operator for debugging
      friend std::ostream& operator<<(std::ostream& output, const Vector3D& vector)
      { output << "Vector3D(" << vector.x() << "," << vector.y() << "," << vector.z() << ")"; return output; }

      /// Calculates the three dimensional dot product.
      inline FloatType dot(const Vector3D& rhs) const { return x() * rhs.x() + y() * rhs.y() + z() * rhs.z(); }

      /// Calculates the two dimensional dot product in xy projection.
      inline FloatType dotXY(const Vector3D& rhs) const { return x() * rhs.x() + y() * rhs.y() ; }

      /// Calculated the three dimensional cross product.
      inline Vector3D cross(const Vector3D& rhs)const {
        return Vector3D(
                 y() * rhs.z() - z() * rhs.y(),
                 z() * rhs.x() - x() * rhs.z(),
                 x() * rhs.y() - y() * rhs.x())
               ;
      }

      /// Calculates the two dimensional cross product in xy projection.
      inline FloatType crossXY(const Vector3D& rhs) const
      { return xy().cross(rhs.xy()); }

      /// Calculates the two dimensional cross product in xy projection.
      inline FloatType crossXY(const Vector2D& rhs) const
      { return xy().cross(rhs); }

      /// Calculates the squared length of the vector
      inline FloatType normSquared() const { return x() * x() + y() * y() + z() * z(); }
      /// Calculates the length of the vector
      inline FloatType norm() const { return hypot(hypot(x(), y()), z()); }





      /** @name Angle functions
       *  These functions measure the angle between two vectors from *this* to rhs. \n
       *  Because the three dimensional space does not have a unique orientation available, \n
       *  the sign of the angle is not well defined. Therefore angles between three dimensional \n
       *  vectors are always positiv and so is the sine of the angle. The angle is bound by PI.
       */
      ///@{
      ///Cosine of the angle between this and rhs
      inline FloatType cosWith(const Vector3D& rhs) const { return dot(rhs) / (norm() * rhs.norm());  }
      ///Sine of the angle between this and rhs
      inline FloatType sinWith(const Vector3D& rhs) const { return cross(rhs).norm() / (norm() * rhs.norm());  }
      ///The angle between this and rhs
      inline FloatType angleWith(const Vector3D& rhs) const { return atan2(sinWith(rhs), cosWith(rhs)); }
      ///@}


      /// Calculates the distance of this point to the rhs
      inline FloatType distance(const Vector3D& rhs = Vector3D(0.0, 0.0, 0.0)) const {
        FloatType deltaX = x() - rhs.x();
        FloatType deltaY = y() - rhs.y();
        FloatType deltaZ = z() - rhs.z();
        return hypot(hypot(deltaX, deltaY), deltaZ);
      }

      /// Scales the vector in place by the given factor
      inline Vector3D& scale(const FloatType& factor)
      { m_xy.scale(factor); m_z *= factor; return *this; }
      /// Same as scale()
      inline Vector3D& operator*=(const FloatType& factor) { return scale(factor); }

      /// Returns a scaled copy of the vector
      inline Vector3D scaled(const FloatType& factor) const
      { return Vector3D(xy().scaled(factor), z() * factor); }

      /// Same as scaled()
      inline Vector3D operator*(const FloatType& factor) const { return scaled(factor); }

      /// Divides all coordinates by a common denominator in place
      inline Vector3D& divide(const FloatType& denominator)
      { m_xy.divide(denominator); m_z /= denominator; return *this; }
      /// Same as divide()
      inline Vector3D& operator/=(const FloatType& denominator) { return divide(denominator); }

      /// Returns a copy where all coordinates got divided by a common denominator
      inline Vector3D divided(const FloatType& denominator) const
      { return Vector3D(xy().divided(denominator), z() / denominator); }
      /// Same as divided()
      inline Vector3D operator/(const FloatType& denominator) const { return divided(denominator); }

      /// Adds a vector to this in place
      inline Vector3D& add(const Vector3D& rhs)     { m_xy.add(rhs.xy()); m_z += rhs.z(); return *this;}
      /// Adds a two dimensional vector to this in place taking z of the given vector as zero
      inline Vector3D& add(const Vector2D& rhs)     { m_xy.add(rhs); return *this; }

      /// Same as add()
      inline Vector3D& operator+=(const Vector3D& rhs) { return add(rhs); }
      /// Same as add()
      inline Vector3D& operator+=(const Vector2D& rhs) { return add(rhs); }

      /// Subtracts a vector to this in place
      inline Vector3D& subtract(const Vector3D& rhs) { m_xy.subtract(rhs.xy()); m_z -= rhs.z(); return *this;}
      /// Subtracts a two dimensional vector from this in place taking z of the given vector as zero
      inline Vector3D& subtract(const Vector2D& rhs) { m_xy.subtract(rhs); return *this;}

      /// Same as subtract()
      inline Vector3D& operator-=(const Vector3D& rhs) { return subtract(rhs); }
      /// Same as subtract()
      inline Vector3D& operator-=(const Vector2D& rhs) { return subtract(rhs); }

      /// Returns a unit vector colaligned with this
      inline Vector3D unit() const { return isNull() ? Vector3D(0.0, 0.0, 0.0) : divided(norm()); }
      /// Normalizes the vector to unit length
      /** Normalizes the vector to unit length and returns the length the vector had before. \n
       *  The null vector is not transformed. */
      inline FloatType normalize()
      { FloatType result = norm(); if (result != 0.0) divide(result); return result; }

      /// Normalizes the vector to the given length
      /** Normalizes the vector to the given length and returns the length the vector had before. \n
       *  The null vector is not transformed. */
      inline FloatType normalizeTo(const FloatType& toLength)
      { FloatType result = norm(); if (result != 0.0) scale(toLength / result); return result; }

      /// Reverses the direction of the vector in place
      inline Vector3D& reverse() { scale(-1.0); return *this; }

      /// Returns a vector pointing in the opposite direction
      inline Vector3D reversed() const { return scaled(-1.0); }

      /// Same as reversed()
      inline Vector3D operator-() const { return reversed(); }

      /// Returns a new vector as sum of this and rhs
      inline Vector3D operator+(const Vector3D& rhs) const
      { return Vector3D(xy() + rhs.xy(), z() + rhs.z()); }

      /// Returns a new vector as differenc of this and rhs
      inline Vector3D operator-(const Vector3D& rhs) const
      { return Vector3D(xy() - rhs.xy(), z() - rhs.z()); }

      /// Calculates the component parallel to the given vector
      inline FloatType parallelComp(const Vector3D& relativTo) const
      { return relativTo.dot(*this) / relativTo.norm(); }

      /// Same as parallelComp() but assumes the given vector to be of unit length.
      /** This assumes the given vector relativeTo to be of unit length and avoids \n
       *  a costly computation of the vector norm()*/
      inline FloatType fastParallelComp(const Vector3D& relativTo) const
      { return relativTo.dot(*this); }

      /// Calculates the component orthogonal to the given vector
      /** The orthogonal component is the rest of the vector not parallel to \n
       *  relative to. Since the three dimensional space does not have a unque \n
       *  orientation given by the vector relative to, the sign of the orthogonal \n
       *  component is meaningless and is always set to positiv */
      inline FloatType orthogonalComp(const Vector3D& relativTo) const
      { return relativTo.cross(*this).norm() / relativTo.norm(); }

      /// Same as orthogonalComp() but assumes the given vector to be of unit length
      /** This assumes the given vector relativeTo to be of unit length and avoids \n
       *  a costly computation of the vector norm()*/
      inline FloatType fastOrthogonalComp(const Vector3D& relativTo) const
      { return relativTo.cross(*this).norm(); }



      /// Getter for the x coordinate
      inline const FloatType& x() const { return m_xy.x(); }
      /// Setter for the x coordinate
      inline void setX(const FloatType& x) { m_xy.setX(x); }
      /// Getter for the y coordinate
      inline const FloatType& y() const { return m_xy.y(); }
      /// Setter for the y coordinate
      inline void setY(const FloatType& y) { m_xy.setY(y); }
      /// Getter for the z coordinate
      inline const FloatType& z() const { return m_z; }
      /// Setter for the z coordinate
      inline void setZ(const FloatType& z) { m_z = z; }

      /// Getter for the xy projected vector ( reference ! )
      inline const Vector2D& xy() const { return m_xy; }
      /// Setter for the xy projected vector
      inline void setXY(const Vector2D& xy) { m_xy = xy; }

      /// Setter for all three coordinates.
      inline void set(const FloatType& first,
                      const FloatType& second,
                      const FloatType& third)
      { setX(first); setY(second); setZ(third);}


      /// Getter for the squared polar radius ( xy projected squared norm )
      inline FloatType polarRSquared() const { return xy().normSquared() ; }
      /// Getter for the polar radius ( xy projected norm )
      inline FloatType polarR() const { return xy().norm(); }
      /// Getter for the polar angle ( xy projected polar angle )
      inline FloatType phi() const { return xy().phi(); }

      //inline FloatType rho() const{ return norm(); }

      /// Getter for the azimuthal angle
      inline FloatType theta() const { return atan2(polarR(), z()); }

      /// Getter for the cotangents of the azimuthal angle
      inline FloatType cotTheta() const { return z() / polarR(); }


    private:
      Vector2D m_xy; ///< Memory for the first and second coordinate available as a vector
      FloatType m_z; ///< Memory for the third coordinate

    private:

      /// ROOT Macro to make Vector3D a ROOT class.
      ClassDefInCDCLocalTracking(Vector3D, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // VECTOR3D_H
