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

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/numerics/ERotation.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A two dimensional circle in its natural representation using center and radius as
    /// parameters.
    class Circle2D {

    public:
      /// Default constructor for ROOT compatibility. Creates an invalid circle
      Circle2D()
        : m_center(0.0, 0.0)
        , m_radius(0.0)
      {
      }

      /// Constructs a circle with given center and radius/ orientation as given by the signedRadius
      Circle2D(const Vector2D& center, const double radius)
        : m_center(center)
        , m_radius(radius)
      {
      }

      /// Constructs a circle with given center, absolut value of the radius and orientation
      Circle2D(const Vector2D& center, const double absRadius, const ERotation ccwInfo)
        : m_center(center)
        , m_radius(fabs(absRadius) * ccwInfo)
      {
      }

    public:
      /// Flips orientation the circle in place
      void reverse()
      {
        m_radius *= -1;
      }

      /// Returns a copy of the line with the reversed orientation
      Circle2D reversed() const
      {
        return Circle2D(center(), -radius());
      }

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
      void conformalTransform()
      {
        double denominator = 1 / (center().normSquared() - radius() * radius());
        m_center *= denominator;
        m_radius *= -denominator;
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
      Circle2D conformalTransformed() const
      {
        double denominator = 1 / (center().normSquared() - radius() * radius());
        return Circle2D(center() * denominator, -radius() * denominator);
      }

    public:
      /// Calculates the signed distance of the point to the circle line.
      double distance(const Vector2D& point) const
      {
        return copysign(center().distance(point), radius()) - radius();
      }

      /// Returns the signed distance to the origin
      double impact() const
      {
        return copysign(center().norm(), radius()) - radius();
      }

      /// Returns the euclidian distance of the point to the circle line
      double absDistance(const Vector2D& point) const
      {
        return fabs(center().distance(point) - absRadius());
      }

      /// Return if the point given is right or left of the line
      ERightLeft isRightOrLeft(const Vector2D& point) const
      {
        return static_cast<ERightLeft>(sign(distance(point)));
      }

      /// Return if the point given is left of the circle line
      bool isLeft(const Vector2D& rhs) const
      {
        return isRightOrLeft(rhs) == ERightLeft::c_Left;
      }

      /// Return if the point given is right of the circle line
      bool isRight(const Vector2D& rhs) const
      {
        return isRightOrLeft(rhs) == ERightLeft::c_Right;
      }

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
      Vector2D tangential() const
      {
        return tangential(Vector2D(0.0, 0.0)).unit();
      }

      /// Gives to azimuth phi of the direction of flight at the perigee
      double tangentialPhi() const
      {
        return tangential().phi();
      }

      /// Gradient of the distance field
      Vector2D gradient(const Vector2D& point) const
      {
        Vector2D connection = (point - center()) * orientation();
        return connection.unit();
      }

      /// Normal vector to the circle near the given position
      Vector2D normal(const Vector2D& point) const
      {
        return gradient(point).unit();
      }

      /// Tangential vector to the circle near the given position
      Vector2D tangential(const Vector2D& point) const
      {
        return normal(point).orthogonal();
      }

      /// Calculates the angle between two points as seen from the center of the circle
      double openingAngle(const Vector2D& from, const Vector2D& to) const
      {
        return gradient(from).angleWith(gradient(to));
      } // can be optimized in the number of computations

      /// Calculates the arc length between two points of closest approach on the circle.
      double arcLengthBetween(const Vector2D& from, const Vector2D& to) const
      {
        return openingAngle(from, to) * radius();
      }

      /// Getter for the signed radius
      double radius() const
      {
        return m_radius;
      }

      /// Getter for the squared radius
      double radiusSquared() const
      {
        return radius() * radius();
      }

      /// Getter for the absolute radius
      double absRadius() const
      {
        return fabs(radius());
      }

      /// Indicates if the circle is to be interpreted counterclockwise or clockwise
      ERotation orientation() const
      {
        return static_cast<ERotation>(sign(radius()));
      }

      /// Getter for the central point of the circle
      Vector2D center() const
      {
        return m_center;
      }

      /** @name Transformations of the circle */
      /**@{*/
      /// Activelly moves the circle in the direction given in place by the vector given
      void moveBy(const Vector2D& by)
      {
        m_center += by;
      }

      /// Activelly moves the circle in the direction given in place along the first coordinate
      void moveAlongFirst(const double first)
      {
        m_center.setFirst(m_center.first() + first);
      }

      /// Activelly moves the circle in the direction given in place along the second coordinate
      void moveAlongSecond(const double second)
      {
        m_center.setSecond(m_center.second() + second);
      }

      /// Passivelly move the coordinate system  in place by the given vector
      void passiveMoveBy(const Vector2D& by)
      {
        m_center -= by;
      }

      /// Passivelly move the coordinate system in place along the first coordinate
      void passiveMoveAlongFirst(const double first)
      {
        m_center.setFirst(m_center.first() - first);
      }

      /// Passivelly move the coordinate system in place along the second coordinate
      void passiveMoveAlongSecond(const double second)
      {
        m_center.setSecond(m_center.second() - second);
      }
      /**@}*/

    private:
      /// Memory for the central point
      Vector2D m_center;

      /// Memory for the signed radius
      double m_radius;

    };
  }
}
