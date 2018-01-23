/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/geometry/GeneralizedCircle.h>
#include <tracking/trackFindingCDC/geometry/Line2D.h>

#include <tracking/trackFindingCDC/geometry/PerigeeParameters.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>
#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/numerics/ERotation.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <utility>
#include <cmath>
#include <iosfwd>

namespace Belle2 {

  namespace TrackFindingCDC {
    class Circle2D;

    /// Extension of the generalized circle also caching the perigee coordinates.
    class PerigeeCircle {

    public:
      /// Default constructor for ROOT compatibility.
      PerigeeCircle();

      /// Constructor from the perigee parammeters. The direction of travel at the perigee is given
      /// as vector.
      PerigeeCircle(double curvature, const Vector2D& phi0Vec, double impact);

      /// Constructor from the perigee parammeters. The direction of travel at the perigee is given
      /// as azimuth angle
      PerigeeCircle(double curvature, double phi0, double impact);

      /// Constructor from the perigee parammeters.
      explicit PerigeeCircle(const PerigeeParameters& perigeeParameters);

    private:
      /// Constructor taking all stored parameters for internal use.
      PerigeeCircle(double curvature, double phi0, const Vector2D& phi0Vec, double impact);

    public:
      /// Constructor from a two dimensional line
      explicit PerigeeCircle(const Line2D& n012);

      /// Constructor promoting the generalized circle
      explicit PerigeeCircle(const GeneralizedCircle& n0123);

      /// Constructor from a two dimensional circle in center / radius representation
      explicit PerigeeCircle(const Circle2D& circle);

      /// Constructor with the four parameters of the generalized circle
      static PerigeeCircle fromN(double n0, double n1, double n2, double n3 = 0);

      /// Constructor with the four parameters of the generalized circle
      static PerigeeCircle fromN(double n0, const Vector2D& n12, double n3 = 0);

      /**
       *  Constructor from center, radius and a optional orientation.
       *
       *  The center and radius alone do not carry any orientation.
       *  However the perigee parameterisation does.
       *  Therefore the constructor also excepts an orientated representation from them.
       *  If no orientation is given, it defaults to mathematical positiv counterclockwise.
       */
      static PerigeeCircle
      fromCenterAndRadius(const Vector2D& center,
                          double absRadius,
                          ERotation orientation = ERotation::c_CounterClockwise);

    public:
      /// Sets all circle parameters to zero
      void invalidate();

      /// Indicates if all circle parameters are zero
      bool isInvalid() const;

      /// Indicates if the combination of the circle parameters makes up a valid circle
      bool isValid() const
      {
        return not isInvalid();
      }

      /// Flips the orientation of the circle in place
      void reverse();

      /// Returns a copy of the circle with opposite orientation.
      PerigeeCircle reversed() const;

    public:
      /**
       *  Transforms the generalized circle to conformal space inplace.
       *
       *  Applies the conformal map in the self-inverse from
       *  X = x / (x^2 + y^2) and Y = y / (x^2 +y^2) inplace
       *  It works most easily by the exchange of the circle parameters n0 <-> n3
       */
      void conformalTransform();

      /**
       *  Returns a copy of the circle in conformal space.
       *
       *  Applies the conformal map in the self-inverse from
       *  X = x / (x^2 + y^2) and Y = y / (x^2 +y^2)
       *  and returns the result as a new circle
       *  It works most easily by the exchange of the circle parameters n0 <-> n3
       */
      PerigeeCircle conformalTransformed() const;

      /// Moves the coordinates system by the given vector. Updates perigee parameters in place
      void passiveMoveBy(const Vector2D& by);

      /// Computes the Jacobi matrix for a move of the coordinate system by the given vector.
      PerigeeJacobian passiveMoveByJacobian(const Vector2D& by) const;

      /// Puts the Jacobi matrix for a move of the coordinate system by the given vector in the
      /// given matrix as an output argument
      void passiveMoveByJacobian(const Vector2D& by, PerigeeJacobian& jacobian) const;

      /// Calculates the point, which lies at the give perpendicular travel distance (counted from
      /// the perigee)
      Vector2D atArcLength(double arcLength) const;

      /// Calculates the arc length between the perigee and the given point.
      double arcLengthTo(const Vector2D& point) const;

      /**
       *  Calculates the arc length between two points of closest approach on the circle.
       *  The arc length is signed positiv for travel in orientation direction.
       *  In the circle case the arc length is between -pi*radius and pi*radius,
       *  hence the discontinuity is on the far side of the circle relative to the given from point.
       *  The points are essentially first taken to their closest approach
       *  before we take the length on the curve.
       *  For the line case the length is the distance component parallel to the line.
       */
      double arcLengthBetween(const Vector2D& from, const Vector2D& to) const;

      /**
       *  Calculates the two dimensional arc length till the cylindrical radius is reached
       *  If the radius can not be reached return NAN.
       *  Note that there are two solutions which have equivalent arc lengths with different sign
       *  Always return the positive solution.
       */
      double arcLengthToCylindricalR(double cylindricalR) const;

      /// Calculates the two points with the given cylindrical radius on the generalised circle
      std::pair<Vector2D, Vector2D> atCylindricalR(double cylindricalR) const;

      /**
       *  Approach on the circle with the given cylindrical radius
       *  that lies in the forward direction of a start point.
       *
       *  Calculates the point on the circle with cylindrical radius cylindricalR,
       *  which is closest following the circle in the direction of positive forward orientation
       *  This is particularly useful to extraplotate into a certain layer.
       *  In case no intersection with this cylindrical radius exists
       *  the function returns Vector2D(NAN,NAN)
       *  @param startPoint Start point from which to follow in the circle in the forward direction
       *  @param cylindricalR Cylindrical radius of interest
       *  @return Close point in forward direction with same cylindrical radius on the circle.
       */
      Vector2D atCylindricalRForwardOf(const Vector2D& startPoint, double cylindricalR) const;

      /// Indicates whether to given point lies in the forward direction from the perigee
      EForwardBackward isForwardOrBackwardOf(const Vector2D& from, const Vector2D& to) const
      {
        return tangential(from).isForwardOrBackwardOf(to - from);
      }

      /// Indicates whether to given point lies in the forward direction from the perigee
      EForwardBackward isForwardOrBackward(const Vector2D& to) const
      {
        return tangential().isForwardOrBackwardOf(to);
      }

      /**
       *  Returns the one of two end point which is first reached from the given start
       *  if one stricly follows the forward direction of the circle.
       *
       *  If the generalized circle is truely a line none of the points might lie
       *  in the forward direction and Vector2D(NAN,NAN) is returned.
       *  @param start Point to start the traversal
       *  @param end1 One possible end point
       *  @param end2 Other possible end point
       *  @return end1 or end2 depending, which lies closer to start in the forward direction or
       *          Vector2D(NAN,NAN) if neither end1 nor end2 are reachable in the forward direction
       *          (line case only)
       */
      Vector2D
      chooseNextForwardOf(const Vector2D& start, const Vector2D& end1, const Vector2D& end2) const;
      /// Calculates the point of closest approach on the circle to the given point.
      Vector2D closest(const Vector2D& point) const;

      /// Getter for the proper signed distance of the point to the circle
      double distance(const Vector2D& point) const
      {
        return distance(fastDistance(point));
      }

      /// Helper function to translate the linearised distance to the proper distance
      double distance(double fastDistance) const;

      /**
       *  Getter for the linearised distance measure to a point.
       *  Gives a fast signed approximation of the distance to the circle.
       *  The absolute value of the fast distance is accurate up to first
       *  order of distance/circle radius, hence best used with big circles and small distances.
       *  The sign of the fast distance indicates if the point is to the right or to the left of the
       *  circle.
       */
      double fastDistance(const Vector2D& point) const;

      /// Getter for the linearised distance to the origin
      double fastImpact() const
      {
        return fastDistance(impact());
      }

      /**
       *  Helper function to translate the proper distance to the linearized distance measure of
       *  the circle retaining the sign of the distance.
       */
      double fastDistance(double distance) const
      {
        return distance * (1.0 + distance * curvature() / 2);
      }

      /// Indicates if the point is on the right or left side of the circle.
      ERightLeft isRightOrLeft(const Vector2D& point) const
      {
        return static_cast<ERightLeft>(sign(fastDistance(point)));
      }

      /// Indicates if the perigee parameters represent a line
      bool isLine() const
      {
        return curvature() == 0.0;
      }

      /// Indicates if the perigee parameters represent a closed circle
      bool isCircle() const
      {
        return curvature() != 0.0;
      }

      /// Getter for the orientation of the circle
      ERotation orientation() const
      {
        return static_cast<ERotation>(sign(curvature()));
      }

      /// Gradient of the distance field, hence indicates the direction of increasing distance.
      Vector2D gradient(const Vector2D& point) const
      {
        return (point - perigee()) * curvature() - phi0Vec().orthogonal();
      }

      /// Unit normal vector from the circle to the given point.
      Vector2D normal(const Vector2D& point) const
      {
        return gradient(point).unit();
      }

      /// Tangential vector to the circle near the given position.
      Vector2D tangential(const Vector2D& point) const
      {
        return normal(point).orthogonal();
      }

      /// Getter for the tangtial vector at the perigee
      const Vector2D& tangential() const
      {
        return phi0Vec();
      }

      /// Getter for the perigee point
      Vector2D perigee() const
      {
        return phi0Vec().orthogonal() * impact();
      }

      /// Getter for the center of the circle. If it was a line both components will be infinity.
      Vector2D center() const
      {
        return phi0Vec().orthogonal() * (impact() + radius());
      }

      /// Getter for the apogee of the circle. If it was a line both components will be infinity.
      Vector2D apogee() const
      {
        return phi0Vec().orthogonal() * (impact() + 2 * radius());
      }

      /// Gives the minimal cylindrical radius the circle reaches (unsigned)
      double minimalCylindricalR() const
      {
        return fabs(impact());
      }

      /// Gives the maximal cylindrical radius the circle reaches
      double maximalCylindricalR() const
      {
        return fabs(impact() + 2 * radius());
      }

      /// Getter for the arc length for a full round of the circle
      double arcLengthPeriod() const
      {
        return std::fabs(perimeter());
      }

      /// Gives the signed perimeter of the circle.
      double perimeter() const
      {
        return 2 * M_PI * radius();
      }

      /// Gives the signed radius of the circle. If it was a line this will be infinity.
      double radius() const
      {
        return 1 / curvature();
      }

      /// Gives the signed radius of the circle. If it was a line this will be infinity.
      double absRadius() const
      {
        return fabs(radius());
      }

      /// Setter for the circle center and radius
      void setCenterAndRadius(const Vector2D& center,
                              double absRadius,
                              ERotation orientation = ERotation::c_CounterClockwise);

      /// Getter for the generalised circle parameter n0
      double n0() const
      {
        return impact() * (impact() * curvature() / 2.0 + 1.0);
      }

      /// Getter for the generalised circle parameters n1 and n2
      Vector2D n12() const
      {
        return -phi0Vec().orthogonal() * (1 + curvature() * impact());
      }

      /// Getter for the generalised circle parameters n1
      double n1() const
      {
        return n12().x();
      }
      //{ return phi0Vec().y() * (1 + curvature() * impact()); }

      /// Getter for the generalised circle parameters n2
      double n2() const
      {
        return n12().y();
      }
      //{ return -(phi0Vec().x()) * (1 + curvature() * impact()); }

      /// Getter for the generalised circle parameter n3
      double n3() const
      {
        return curvature() / 2.0;
      }

      /// Setter for four generalised circle parameters.
      void setN(double n0, double n1, double n2, double n3 = 0.0)
      {
        setN(n0, Vector2D(n1, n2), n3);
      }

      /// Setter for four generalised circle parameters.
      void setN(double n0, const Vector2D& n12, double n3 = 0.0);

      /// Setter for generalised circle parameters from a normal line.
      void setN(const Line2D& n012)
      {
        setN(n012.n0(), n012.n12());
      }

      /// Setter for four generalised circle parameters.
      void setN(const GeneralizedCircle& n0123)
      {
        setN(n0123.n0(), n0123.n12(), n0123.n3());
      }

      /// Getter for omega parameter of the common Belle2::Helix which is the wrong sign curvature
      double omega() const
      {
        return -curvature();
      }

      /// Getter for d0 parameter of the common Belle2::Helix representation
      double d0() const
      {
        return -impact();
      }

      /// Getter for the signed curvature.
      double curvature() const
      {
        return m_curvature;
      }

      /// Getter for the azimuth angle of the direction of flight at the perigee
      double phi0() const
      {
        return m_phi0;
      }

      /// Getter for the unit vector of the direction of flight at the perigee
      const Vector2D& phi0Vec() const
      {
        return m_phi0Vec;
      }

      /// Getter for the signed distance of the origin to the circle.
      double impact() const
      {
        return m_impact;
      }

      /// Getter for the three perigee parameters in the order defined by EPerigeeParameter.h
      PerigeeParameters perigeeParameters() const
      {
        using namespace NPerigeeParameterIndices;
        PerigeeParameters result;
        result(c_Curv) = curvature();
        result(c_Phi0) = phi0();
        result(c_I) = impact();
        return result;
      }

      /// Setter for signed curvature.
      void setCurvature(double curvature)
      {
        m_curvature = curvature;
      }

      /// Sets the azimuth angle of the direction of flight at the perigee.
      void setPhi0(double phi0)
      {
        m_phi0 = phi0;
        m_phi0Vec = Vector2D::Phi(phi0);
      }

      /// Sets the unit direction of flight at the perigee
      void setPhi0(const Vector2D& phi0Vec)
      {
        m_phi0 = phi0Vec.phi();
        m_phi0Vec = phi0Vec.unit();
      }

      /// Sets the impact parameter of the circle.
      void setImpact(double impact)
      {
        m_impact = impact;
      }

      /// Setter for the perigee parameters
      void setPerigeeParameters(double curvature, const Vector2D& phi0Vec, double impact)
      {
        m_impact = impact;
        m_phi0 = phi0Vec.phi();
        m_phi0Vec = phi0Vec.unit();
        m_curvature = curvature;
      }

      /// Setter for the perigee parameters
      void setPerigeeParameters(double curvature, double phi0, double impact)
      {
        m_impact = impact;
        m_phi0 = phi0;
        m_phi0Vec = Vector2D::Phi(phi0);
        m_curvature = curvature;
      }

    private:
      /// Helper method to calculate the arc length to a point at distance  delta to the perigee and dr to circle.
      double arcLengthAtDeltaLength(double delta, double dr) const;

      /// Helper method to calculate the arc length between to points on the circle from a given direct secant length
      double arcLengthAtSecantLength(double secantLength) const;

    private:
      /// Memory for the signed curvature
      double m_curvature;

      /// Memory for the azimuth angle of the direction of flight at the perigee
      double m_phi0;

      /// Cached unit direction of flight at the perigee
      Vector2D m_phi0Vec;

      /// Memory for the signed impact parameter
      double m_impact;

    };

    /// Debug helper
    std::ostream& operator<<(std::ostream& output, const PerigeeCircle& circle);
  }
}
