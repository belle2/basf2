/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <tracking/trackFindingCDC/geometry/Vector2D.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /// A two dimensional circle in its natural representation
    /** An oriented two dimensional circle in the central point, radius representation.
     *  The circle is oriented by using the sign of the radius variable.
     *  The convension is that positiv radii correspond to a counterclockwise orientation of the circle,
     *  while negative radii are for the clockwise orientation.
     */
    class Circle2D  {

    public:

      /// Default constructor for ROOT compatibility. , Constructs the unit circle around the origin
      Circle2D():
        m_center(0.0, 0.0),
        m_radius(0.0)
      {}

      /// Constructs a circle with given center and radius/ orientation as given by the signedRadius
      Circle2D(const Vector2D& center, const double radius):
        m_center(center),
        m_radius(radius)
      {}

      /// Constructs a circle with given center, absolut value of the radius and orientation
      Circle2D(const Vector2D& center, const double absRadius, const ERotation ccwInfo):
        m_center(center),
        m_radius(fabs(absRadius) * ccwInfo)
      {}

    public:
      /// Flips orientation the circle in place
      void reverse()
      { m_radius *= -1; }

      /// Returns a copy of the line with the reversed orientation
      Circle2D reversed() const
      { return Circle2D(center(), -radius()); }

      /**
       *  Transforms the circle to conformal space inplace.
       *
       *  Applies the conformal map in the self-inverse from
       *  - \f$X = x / (x^2 + y^2 - r^2)\f$
       *  - \f$Y = y / (x^2 + y^2 - r^2)\f$
       *  - \f$R = r / (x^2 + y^2 - r^2)\f$
       *  inplace
       *  This is only stable for off origin circles. The numerical stability of the transformation
       *  is subjected to the denominator center().normSquared() - signedRadius() * signedRadius().
       **/
      inline void conformalTransform()
      {
        double denominator = 1 / (center().normSquared() - radius() * radius());
        m_center *= denominator;
        m_radius *= denominator;
      }

      /**
       *  Returns a copy of the circle in conformal space.
       *
       *  Applies the conformal map in the self-inverse from
       *  - \f$X = x / (x^2 + y^2 - r^2)\f$
       *  - \f$Y = y / (x^2 + y^2 - r^2)\f$
       *  - \f$R = r / (x^2 + y^2 - r^2)\f$
       *  and returns the result as new Circle2D
       *  This is only stable for off origin circles. The numerical stability of the transformation
       *  is subjected to the denominator center().normSquared() - signedRadius() * signedRadius().
       **/
      inline Circle2D conformalTransformed() const
      {
        double denominator = 1 / (center().normSquared() - radius() * radius());
        return Circle2D(center() * denominator, radius() * denominator);
      }

    public:
      /// Calculates the signed distance of the point to the circle line.
      /** Returns the signed distance of the point to the line. The sign is positiv \n
       *  for the right side of the line and negativ for the left side. */
      double distance(const Vector2D& point) const
      { return copysign(center().distance(point), radius()) - radius(); }

      /// Returns the signed distance to the origin
      /** The distance to the origin is equivalent to the first line parameter*/
      double impact() const
      { return copysign(center().norm(), radius()) - radius(); }

      /// Returns the euclidian distance of the point to the circle line
      double absDistance(const Vector2D& point) const
      { return fabs(center().distance(point) - absRadius()); }

      /// Return if the point given is right or left of the line
      ERightLeft isRightOrLeft(const Vector2D& point) const
      { return static_cast<ERightLeft>(sign(distance(point))); }

      /// Return if the point given is left of the circle line
      inline bool isLeft(const Vector2D& rhs) const
      { return isRightOrLeft(rhs) == ERightLeft::c_Left; }

      /// Return if the point given is right of the circle line
      inline bool isRight(const Vector2D& rhs) const
      { return isRightOrLeft(rhs) == ERightLeft::c_Right; }

      /// Calculates the point of closest approach on the line to the point
      Vector2D closest(const Vector2D& point) const
      {
        Vector2D connection = point - center();
        connection.normalizeTo(absRadius());
        connection += center();
        return connection;
      }

      /// Returns the point closest to the origin
      Vector2D perigee() const
      {
        Vector2D connection = center();
        connection.normalizeTo(-absRadius());
        connection += center();
        return connection;
      }

      /// Gives the tangential vector at the closest approach to the origin / at the perigee
      inline Vector2D tangential() const
      { return tangential(Vector2D(0.0, 0.0)).unit(); }


      /// Gives to azimuth phi of the direction of flight at the perigee
      inline double tangentialPhi() const
      { return tangential().phi(); }

      /// Gradient of the distance field
      /**
       * Gives the gradient of the euclidian distance field for the given point.
       * @param point Point in the plane to calculate the gradient
       * @return Gradient of the distance field
       */
      inline Vector2D gradient(const Vector2D& point) const
      {
        Vector2D connection = (point - center()) * orientation();
        return connection.unit();
      }

      /// Normal vector to the circle near the given position
      /**
       * Gives the unit normal vector to the circle line.
       * It points to the outside of the circle for counterclockwise orientation,
       * and inwards for the clockwise orientation.
       * It is essentially the gradient normalized to unit length.
       * @param point Point in the plane to calculate the tangential
       * @return Unit normal vector to the circle line
       */
      inline Vector2D normal(const Vector2D& point) const
      { return gradient(point).unit(); }

      /// Tangential vector to the circle near the given position
      /**
       * Gives the unit tangential vector to the circle line.
       * It always points in the direction of positiv advance
       * at the point of closest approach from the given point.
       * @param point Point in the plane to calculate the tangential
       * @return Unit tangential vector to the circle line
       */
      inline Vector2D tangential(const Vector2D& point) const
      { return normal(point).orthogonal(); }

      /// Calculates the angle between two points of closest approach on the circle
      /**
       * The angle is signed positiv for a counterclockwise rotation.
       * The points are essentially first taken to their closest approach
       * before we take the opening angle as seen from the circle center.
       * The angle will zero if the generalized circle was line.
       */
      inline double openingAngle(const Vector2D& from, const Vector2D& to) const
      { return gradient(from).angleWith(gradient(to)); } //can be optimized in the number of computations


      /// Calculates the arc length between two points of closest approach on the circle.
      /**
       * The arc length is signed positiv for travel in orientation direction
       * The points are essentially first taken to their closest approach
       * before we take the length on the curve.
       * For the line case the length is the distance component parallel to the line.
       */
      inline double lengthOnCurve(const Vector2D& from, const Vector2D& to) const
      { return openingAngle(from, to) * radius(); }

      /// Getter for the signed radius
      /** The sign encodes the orientation of the circle */
      double radius() const
      { return m_radius; }

      /// Getter for the square radius
      /** The sign encodes the orientation of the circle */
      double radiusSquared() const { return radius() * radius(); }

      /// Getter for the absolute radius
      /** The absolute radius is the absolute value of the signed Radius */
      double absRadius() const
      { return fabs(radius()); }

      /// Indicates if the circle is oriented counterclockwise
      // bool isERotation::c_CounterClockwise() const
      // { return isERotation::c_CounterClockwiseOrERotation::c_Clockwise == ERotation::c_CounterClockwise; }

      // /// Indicates if the circle is oriented clockwise
      // bool isERotation::c_Clockwise() const
      // { return isERotation::c_CounterClockwiseOrERotation::c_Clockwise() == ERotation::c_Clockwise; }

      // /// Indicates if the circle is oriented counterclockwise or clockwise
      // ERotation isERotation::c_CounterClockwiseOrERotation::c_Clockwise() const
      // { return sign(radius()); }

      /// Indicates if the circle is to be interpreted counterclockwise or clockwise
      ERotation orientation() const
      { return static_cast<ERotation>(sign(radius())); }

      /// Getter for the central point of the circle
      Vector2D center() const
      { return m_center; }

      /** @name Transformations of the circle */
      /**@{*/
      /// Activelly moves the circle in the direction given in place by the vector given
      void moveBy(const Vector2D& by)
      { m_center += by; }

      /// Activelly moves the circle in the direction given in place along the first coordinate
      void moveAlongFirst(const double first)
      { m_center.setFirst(m_center.first() + first); }

      /// Activelly moves the circle in the direction given in place along the second coordinate
      void moveAlongSecond(const double second)
      { m_center.setSecond(m_center.second() + second); }

      /// Passivelly move the coordinate system  in place by the given vector
      void passiveMoveBy(const Vector2D& by)
      { m_center -= by; }

      /// Passivelly move the coordinate system in place along the first coordinate
      void passiveMoveAlongFirst(const double first)
      { m_center.setFirst(m_center.first() - first); }

      /// Passivelly move the coordinate system in place along the second coordinate
      void passiveMoveAlongSecond(const double second)
      { m_center.setSecond(m_center.second() - second); }
      /**@}*/

    private:
      Vector2D  m_center; ///< Memory for the central point
      double m_radius; ///< Memory for the signed radius

    }; //class
  } // namespace TrackFindingCDC
} // namespace Belle2
