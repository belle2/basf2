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

#include <tracking/trackFindingCDC/geometry/GeneralizedCircle.h>
#include <tracking/trackFindingCDC/geometry/Line2D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/Quadratic.h>
#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>
#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/numerics/ERotation.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <utility>
#include <cmath>
#include <iosfwd>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A line with a support point and tangential vector
    /** Implements the representation \f$ x = s + \lambda \cdot t \f$ where \n
     *  \f$ s \f$ is the support point and  \f$ t \f$ is the tangential vector \n
     *  \f$ \lambda \f$ is called the parameter of the line. In the same sense as the Line2D\n
     *  this line defines a direction of positiv advance ( the tangential vector ) as well as \n
     *  a right and a left half plane.
     */
    class ParameterLine2D {

    public:
      /// Default constructor for ROOT compatibility.
      ParameterLine2D()
        : m_support(0.0, 0.0)
        , m_tangential(0.0, 0.0)
      {
      }

      /// Standard constructor taking the support point and the tangential vector
      ParameterLine2D(const Vector2D& support, const Vector2D& tangential)
        : m_support(support)
        , m_tangential(tangential)
      {
      }

      /// Constructs a line with slope and intercept.
      /** Orientation will be coaligned with the first axes.
       *  Tangential vector is normalized to have a unit in the first coordinate.
       */
      static ParameterLine2D fromSlopeIntercept(const double slope, const double intercept)
      {
        return ParameterLine2D(Vector2D(0.0, intercept), Vector2D(1.0, slope));
      }

      /// Constructs a line with slope and intercept. Orientation means the alignment with the first
      /// axes.
      /** Orientation will be coaligned or antialigned as given.
       *  Tangential vector is normalized to have a unit in the first coordinate.
       */
      static ParameterLine2D fromSlopeIntercept(const double slope,
                                                const double intercept,
                                                const EForwardBackward orientation)
      {
        return ParameterLine2D(Vector2D(0.0, intercept),
                               Vector2D(orientation, orientation * slope));
      }

      /// Static constructor for a line between to points
      /** The resulting line will have start as support. Tangential vector is the difference between
       * end and start.
       *  Hence at(0) == start and at(1) == end for the constructed line.
       */
      static ParameterLine2D throughPoints(const Vector2D& start, const Vector2D& end)
      {
        return ParameterLine2D(start, end - start);
      }

      /// Upcast the normal representation to a parameter line.
      /** Upcasting requires some additional information to be set namely the support point and the
       * scale of the tangential.
       *  The support is set the point closest to the origin. The tangential is of unit length.
       *  Note the orientation of the line remains the same.
        */
      explicit ParameterLine2D(const Line2D& line)
        : m_support(line.support())
        , m_tangential(line.tangential())
      {
      }

      /// Constructs a line touching two circles in one point each.
      /** @param[in] fromCenter first circle center
       *  @param[in] fromSignedRadius radius of the first circle multiplied with the right left
       * passage information
       *  @param[in] toCenter second circle center
       *  @param[in] toSignedRadius radius of the first circle multiplied with the right left
       * passage information.
       *  @return the line being tangential to both circles.
       *  Note : the touch points reside at(0) for the first and at(1) for the second */
      static ParameterLine2D touchingCircles(const Vector2D& fromCenter,
                                             double fromSignedRadius,
                                             const Vector2D& toCenter,
                                             double toSignedRadius);

      /// Downcast the line to the normal representation.
      /** Downcasting the parameter line to the line representation using the normal vector.
       *  You loose the information where the support point is and the scale of the parameter.
       *  However the same points lie on the line and distance to is remain the same
       */
      operator Line2D()
      {
        return Line2D(distanceToOrigin(), normal().unit());
      } // not optimal yet. tangential.norm() is getting calculated two times.

    public:
      /// Gives the tangential vector of the line.
      const Vector2D& tangential() const
      {
        return m_tangential;
      }
      /// Gives the normal vector of the line.
      Vector2D normal() const
      {
        return tangential().orthogonal(ERotation::c_Clockwise);
      }

      /// Gives the support vector of the line.
      const Vector2D& support() const
      {
        return m_support;
      }

      /// Evaluates the line formula at the parameter given
      Vector2D at(const double parameter) const
      {
        return tangential() * parameter += support();
      }

      /// Indicates if the tangential vector point in a commmon direction with the first coordinate
      /// axes
      EForwardBackward alignedWithFirst() const
      {
        return static_cast<EForwardBackward>(sign(tangential().first()));
      }

      /// Indicates if the tangential vector point in a commmon direction with the second coordinate
      /// axes
      EForwardBackward alignedWithSecond() const
      {
        return static_cast<EForwardBackward>(sign(tangential().second()));
      }

      /// Normalizes the tangential vector inplace
      /** Normalizes the line representation such that the parameter is indentical with the distance
       *  from the support point of a point on the line. This can be usefull if you want to give the
       *  parameter a certain meaning where you have to get rid of the proportional factor assizated
       *  with the length of the tangential.
       */
      void normalize()
      {
        m_tangential.normalize();
      }

      /// Clear all information from the line
      void invalidate()
      {
        m_support.set(0, 0);
        m_tangential.set(0, 0);
      }

      /// Check it the line is in an invalid state.
      bool isInvalid() const
      {
        return m_tangential.isNull();
      }

      /// Reverses the tangential vector inplace
      void reverse()
      {
        m_tangential.reverse();
      }

      /// Makes a copy line which has the opposite tangential vector but same support point.
      ParameterLine2D reversed() const
      {
        return ParameterLine2D(support(), -tangential());
      }

    public:
      /// Gives the signed distance of a point to the line
      /** Returns the signed distance of the point to the line. The sign is positiv \n
       *  for the right side of the line and negativ for the left side. */
      double distance(const Vector2D& point) const
      {
        return distanceToOrigin() - point.orthogonalComp(tangential());
      }

      /// Calculates the signed distance of the point given by its to coordinates to the line.
      /** Returns the signed distance of the point to the line. The sign is positiv \n
       *  for the right side of the line and negativ for the left side. */
      double distance(const double first, const double second) const
      {
        return distance(Vector2D(first, second));
      }

      /// Gives the signed distance of the origin
      double distanceToOrigin() const
      {
        return support().orthogonalComp(tangential());
      }

      /// Gives the unsigned distance of a point to the line
      double absoluteDistance(const Vector2D& point) const
      {
        return fabs(distance(point));
      }

      /// Return if the point given is right or left of the line
      ERightLeft isRightOrLeft(const Vector2D& point) const
      {
        return static_cast<ERightLeft>(sign(distance(point)));
      }

      /// Return if the point given is left of the line
      bool isLeft(const Vector2D& rhs) const
      {
        return isRightOrLeft(rhs) == ERightLeft::c_Left;
      }

      /// Return if the point given is right of the line
      bool isRight(const Vector2D& rhs) const
      {
        return isRightOrLeft(rhs) == ERightLeft::c_Right;
      }

      /// Gives the position at the closest approach on the line to point
      Vector2D closest(const Vector2D& point) const
      {
        double norm_squared = tangential().normSquared();
        return Vector2D(tangential(),
                        tangential().dot(point) / norm_squared,
                        tangential().cross(support()) / norm_squared);
      }

      /// Gives the line parameter at the closest approach to point
      double closestAt(const Vector2D& point) const
      {
        return (tangential().dot(point) - tangential().dot(support())) / tangential().normSquared();
      }

      /// Gives the position of closest approach to the origin
      Vector2D closestToOrigin() const
      {
        return tangential().orthogonal() *=
                 (tangential().cross(support()) / tangential().normSquared());
      }

      /// Gives the line parameter at the closest approach to the origin
      double closestToOriginAt() const
      {
        return -tangential().dot(support()) / tangential().normSquared();
      }

      /// Denotes the length on the line between the two points
      double lengthOnCurve(const Vector2D& from, const Vector2D& to) const
      {
        return (to.dot(tangential()) - from.dot(tangential())) / tangential().norm();
      }

      /// Gives the line parameter where the two lines meet. Infinity for parallels.
      double intersectionAt(const Line2D& line) const
      {
        return -(line.n0() + support().dot(line.normal())) / tangential().dot(line.normal());
      }

      /// Gives the line parameter of this line where the two lines meet. Infinity for parallels.
      double intersectionAt(const ParameterLine2D& line) const
      {
        return (line.tangential().cross(support()) - line.tangential().cross(line.support())) /
               tangential().cross(line.tangential());
      }

      /// Gives the line parameters of this line, where it intersects with the generalized circle
      /** Calculates the two line parameters of the intersections with the circle as a pair.
      The second of the pair is always the small of the two solutions, which is closer to the
      support point.
      */
      std::pair<double, double> intersectionsAt(const GeneralizedCircle& genCircle) const
      {
        double a = genCircle.n3() * tangential().normSquared();
        double b = tangential().dot(genCircle.gradient(support()));
        double c = genCircle.fastDistance(support());

        return solveQuadraticABC(a, b, c);
      }

      /// Gives the point where the two lines meet. Infinities for parallels.
      Vector2D intersection(const Line2D& line) const
      {
        return at(intersectionAt(line));
      }

      /// Gives the point where the two lines meet. Infinities for parallels.
      Vector2D intersection(const ParameterLine2D& line) const
      {
        return at(intersectionAt(line));
      }

      /** @name Transformations of the line */
      /**@{*/
      /// Moves the support point by the given amount of the parameter in the forward direction
      /** This moves the parameter by -delta and the following relation hold \n
       *  old at(p) = new at(p - delta) hence the coordinate at is diminished \n
       *  This corresponds to a passive movement of the coordinate system on the line */
      void passiveMoveAtBy(const double delta)
      {
        m_support += tangential() * delta;
      }

      /// Moves the line in the given direction in place. Corresponds to an active transformation.
      void moveBy(const Vector2D& by)
      {
        m_support += by;
      }

      /// Moves the line along the first coordinate axes in place. Corresponds to an active
      /// transformation.
      void moveAlongFirst(const double first)
      {
        m_support.setFirst(m_support.first() + first);
      }

      /// Moves the line along the second coordinate axes in place. Corresponds to an active
      /// transformation.
      void moveAlongSecond(const double second)
      {
        m_support.setSecond(m_support.second() + second);
      }

      /// Moves the coordinate system in the given direction  in place. Corresponds to a passive
      /// transformation.
      void passiveMoveBy(const Vector2D& by)
      {
        m_support -= by;
      }

      /// Moves the coordinate system along the first coordinate axes in place. Corresponds to a
      /// passive transformation.
      void passiveMoveAlongFirst(const double first)
      {
        m_support.setFirst(m_support.first() - first);
      }

      /// Moves the coordinate system along the second coordinate axes in place. Corresponds to a
      /// passive transformation.
      void passiveMoveAlongSecond(const double second)
      {
        m_support.setSecond(m_support.second() - second);
      }
      /**@}*/

      /** @name Line as a function of the first coordinate
       * The next couple of methods are to interpret the line as a function mapping from first to
       * second coordinate */
      /**@{*/

      /// The line slope
      double slope() const
      {
        return tangential().second() / tangential().first();
      }

      /// The inveres line slope
      double inverseSlope() const
      {
        return tangential().first() / tangential().second();
      }

      /// Second coordinate for first being zero
      double intercept() const
      {
        return support().second() - slope() * support().first();
      }

      /// First coordinate for second being zero
      double zero() const
      {
        return support().first() - inverseSlope() * support().second();
      }

      /// Method mapping the first coordinate to the second according to the line
      double map(const double first) const
      {
        return support().second() + slope() * (first - support().first());
      }

      /// Operator mapping the first coordinate to the second according to the line
      double operator()(const double first) const
      {
        return map(first);
      }

      /// Method for the inverse mapping the second coordinate to the first according to the line
      double inverseMap(const double second) const
      {
        return support().first() + inverseSlope() * (second - support().second());
      }

      /// Turns the line into its inverse function in place. Orientation will be flipped as well
      void invert()
      {
        m_tangential.swapCoordinates();
        m_support.swapCoordinates();
      }

      /// Gives the line assoziated with the inverse function as a copy.
      ParameterLine2D inverted() const
      {
        return ParameterLine2D(Vector2D(support().second(), support().first()),
                               Vector2D(tangential().second(), tangential().first()));
      }
      ///@}

    private:
      /// Support vector of the line
      Vector2D m_support;

      /// Tangential vector of the line
      Vector2D m_tangential;
    };

    /// Output operate helping debugging.
    std::ostream& operator<<(std::ostream& output, const ParameterLine2D& line);
  }
}
