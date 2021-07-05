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

#include <tracking/trackFindingCDC/geometry/Line2D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>
#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/numerics/ERotation.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <utility>
#include <iosfwd>
#include <cmath>

namespace Belle2 {

  namespace TrackFindingCDC {
    class Circle2D;

    /**
     *  A generalized circle.
     *
     *  Makes a smooth generalization from a two dimensional normal line
     *  ( like Line2D ) to a circle.
     *
     *  The parameterisation is best suited for low curvature circle. The representation takes four
     *  parameters. They correspond to the normal circle parameters like \n
     *  n0 = (m_x*m_x + m_y*m_y - r*r)/2r \n
     *  n1 = -m_x/r \n
     *  n2 = -m_y/r \n
     *  n3 = 1/2r \n
     *  where the normalization condtion is n1*n1 + n2*n2 - 4 * n0 * n3 = 1.
     *  The overall sign is fixed in the following way:
     *  If the last parameter is positiv the circle is assummed to be orientated counterclockwise
     *  else the circle is assummed to be orientated clockwise.
     *  The parameters n1 and n2 are indeed a vector in two dimensions and
     *  we keep them stored as Vector2D.
     *  Additionally we can represent a line with same parameters by setting n3 = 0. Compare Line2D.
     */
    class GeneralizedCircle {

    public:
      /// Default constructor for ROOT compatibility.
      GeneralizedCircle();

      /// Constructor with the four parameters of the generalized circle
      GeneralizedCircle(double n0, double n1, double n2, double n3 = 0);

      /// Constructor with the four parameters of the generalized circle
      GeneralizedCircle(double n0, const Vector2D& n12, double n3 = 0);

      /// Constructor from a two dimensional line
      explicit GeneralizedCircle(const Line2D& n012);

      /// Constructor from a two dimensional circle
      explicit GeneralizedCircle(const Circle2D& circle);

      /**
       *  Constructor from center, radius and a optional orientation.
       *  The center and radius alone do not carry any orientation.
       *  However the generalized circle does.
       *  This constructor makes an orientated representation from them.
       *  If not given the orientation defaults to
       *  mathematical positiv counterclockwise.
       */
      static GeneralizedCircle
      fromCenterAndRadius(const Vector2D& center,
                          double absRadius,
                          ERotation orientation = ERotation::c_CounterClockwise);

      /**
       *  Constructor of a generalized circle from perigee parameters.
       *  Tangential at perigee given as two dimensional vector.
       */
      static GeneralizedCircle
      fromPerigeeParameters(double curvature, const Vector2D& tangential, double impact);

      /**
       *  Constructor of a generalized circle from perigee parameters.
       *  Tangential at perigee given as azimuth angle.
       */
      static GeneralizedCircle
      fromPerigeeParameters(double curvature, double tangentialPhi, double impact);

    protected:
      /**
       *  Setter for first circle parameter.
       *  Makes _no_ normalization after setting.
       *  Use is discouraged.
       */
      void setN0(const double n0)
      {
        m_n0 = n0;
      }

      /**
       *  Setter for second circle parameter.
       *  Makes _no_ normalization after setting.
       *  Use is discouraged.
       */
      void setN1(const double n1)
      {
        m_n12.setX(n1);
      }

      /**
       *  Setter for third circle parameter.
       *  Makes _no_ normalization after setting.
       *  Use is discouraged.
       */
      void setN2(const double n2)
      {
        m_n12.setY(n2);
      }

      /**
       *  Setter for second and third circle parameter.
       *  Makes _no_ normalization after setting.
       *  Use is discouraged.
       */
      void setN12(const double n1, const double n2)
      {
        m_n12.setXY(n1, n2);
      }

      /**
       *  Setter for second and third circle parameter.
       *  Makes _no_ normalization after setting.
       *  Use is discouraged.
       */
      void setN12(const Vector2D& n12)
      {
        m_n12.setXY(n12);
      }

      /**
       *  Setter for fourth circle parameter.
       *  Makes _no_ normalization after setting.
       *  Use is discouraged.
       */
      void setN3(const double n3)
      {
        m_n3 = n3;
      }

    public:
      /// Setter for the circle center and radius
      void setCenterAndRadius(const Vector2D& center,
                              double absRadius,
                              ERotation orientation = ERotation::c_CounterClockwise);

      /// Setter for the perigee parameters
      void setPerigeeParameters(double curvature, const Vector2D& tangential, double impact);

      /// Setter for the perigee parameters
      void
      setPerigeeParameters(const double curvature, const double tangentialPhi, const double impact)
      {
        setPerigeeParameters(curvature, Vector2D::Phi(tangentialPhi), impact);
      }

      /**
       *  Setter for all four circle parameters.
       *  Makes a normalization after setting.
       *  The normal representation of a line leave out the last parameter.
       */
      void setN(const double n0, const double n1, const double n2, const double n3 = 0.0)
      {
        setN0(n0);
        setN12(n1, n2);
        setN3(n3);
        normalize();
      }

      /**
       *  Setter for all four circle parameters.
       *  Makes a normalization after setting.
       *  The normal representation of a line leave out the last parameter.
       */
      void setN(const double n0, const Vector2D& n12, const double n3 = 0.0)
      {
        setN0(n0);
        setN12(n12);
        setN3(n3);
        normalize();
      }

      /// Setter for all four circle parameters from another circle
      void setN(const Line2D& n012)
      {
        setN(n012.n0(), n012.n12());
      }

      /// Setter for all four circle parameters from another circle
      void setN(const GeneralizedCircle& n0123)
      {
        setN(n0123.n0(), n0123.n12(), n0123.n3());
      }

      /// Sets all circle parameters to zero
      void invalidate()
      {
        setN(0.0, 0.0, 0.0, 0.0);
      }

      /// Flips the orientation of the circle in place
      void reverse()
      {
        scaleN(-1);
      }

      /**
       *  Transforms the generalized circle to conformal space inplace
       *  Applies the conformal map in the self-inverse form
       *  X = x / (x^2 + y^2) and Y = y / (x^2 +y^2) inplace
       *  It works most easily by the exchange of the circle parameters n0 <-> n3
       */
      void conformalTransform()
      {
        std::swap(m_n0, m_n3);
        reverse(); // Correct orientation
      }

      /**
       *  Moves the coordinate system by the given vector.
       *  Updates the circle parameters inplace.
       */
      void passiveMoveBy(const Vector2D& by)
      {
        setN(fastDistance(by), gradient(by), n3());
      }

    protected:
      /**
       *  Normalizes the circle parameters.
       *
       *  The normalization is only made if the circle parameters have a valid positiv
       *  norm. If the normalization of the cirlce is negativ or zero all circle
       *  parameters are not changed.
       */
      void normalize()
      {
        double normalization_squared = normalizationSquared();
        if (normalization_squared > 0) scaleN(1.0 / std::sqrt(normalization_squared));
      }

    private:
      /// Scales the circle parameters by a common factor.
      void scaleN(const double factor)
      {
        m_n0 *= factor;
        m_n12 *= factor;
        m_n3 *= factor;
      }

    public:
      /// Getter for the first circle parameter
      double n0() const
      {
        return m_n0;
      }

      /// Getter for the second circle parameter
      double n1() const
      {
        return m_n12.x();
      }

      /// Getter for the third circle parameter
      double n2() const
      {
        return m_n12.y();
      }

      /// Getter for the second and third circle parameter which natuarally from a vector
      const Vector2D& n12() const
      {
        return m_n12;
      }

      /// Getter for the fourth circle parameter
      double n3() const
      {
        return m_n3;
      }

    public:
      /// Indicates if all circle parameters are zero
      bool isInvalid() const
      {
        return n0() == 0 and n12().isNull() and n3() == 0;
      }

      /// Indicates if the combination of the circle parameters makes up a valid circle
      bool isValid() const
      {
        return not isInvalid();
      }

      /**
       *  Calculates the generalized circle specific squared norm.
       *  Correctly normalized this should give one.
       */
      double normalizationSquared() const
      {
        return n12().normSquared() - 4 * n0() * n3();
      }

    public:
      /// Returns a copy of the circle with opposite orientation.
      GeneralizedCircle reversed() const
      {
        return GeneralizedCircle(-n0(), -n12(), -n3());
      }

    public:
      /**
       *  Returns a copy of the circle in conformal space.
       *  Applies the conformal map in the self-inverse from
       *  X = x / (x^2 + y^2) and Y = y / (x^2 +y^2) and
       *  returns the result as a new GeneralizedCircle.
       *  It works most easily by the exchange of the circle parameters n0 <-> n3
       */
      GeneralizedCircle conformalTransformed() const
      {
        return GeneralizedCircle(-n3(), -n12(), -n0());
      }

    public:
      /**
       *  Gradient of the distance field
       *  Gives the gradient of the approximated distance field for the given point.
       *  @param point Point in the plane to calculate the gradient
       *  @return Gradient of the distance field
       */
      Vector2D gradient(const Vector2D& point) const
      {
        return point * (2.0 * n3()) + n12();
      }

      /**
       *  Normal vector to the circle near the given position.
       *  Gives the unit normal vector to the circle line.
       *  It points to the outside of the circle for counterclockwise orientation,
       *  and inwards for the clockwise orientation.
       *  It is essentially the gradient normalized to unit length
       *  @param point Point in the plane to calculate the tangential
       *  @return Unit normal vector to the circle line
       */
      Vector2D normal(const Vector2D& point) const
      {
        return gradient(point).unit();
      }

      /**
       *  Tangential vector to the circle near the given position.
       *  Gives the unit tangential vector to the circle line.
       *  It always points in the direction of positiv advance
       *  at the point of closest approach from the given point.
       *  @param point Point in the plane to calculate the tangential
       *  @return Unit tangential vector to the circle line
       */
      Vector2D tangential(const Vector2D& point) const
      {
        return normal(point).orthogonal();
      }

      /**
       *  Closest approach on the circle to the point.
       *  Calculates the closest point on the circle relative to the given point.
       *  @param point Point in the plane to calculate the closest approach to
       *  @return Point of closest approach on the circle.
       */
      Vector2D closest(const Vector2D& point) const;

      /**
       *  Calculates the closest approach to the two dimensional origin.
       */
      Vector2D perigee() const;

      /**
       *  Calculates the point on the circle that is furthest away from the origin.
       *  This results in Vector2D(NAN, NAN) in the straight line case.
       */
      Vector2D apogee() const;

      /**
       *  Calculates if the to vector is closer to the from vector
       *  following the along orientation of the circle or against.
       *  @returns
       *      * EForwardBackward::c_Forward in case the to vector is closer following the along the
       * orientation
       *      * EForwardBackward::c_Backward in case the to vector is closer against the
       * orientation.
       *      * EForwardBackward::c_Unknown if neither can be determined.
       */
      EForwardBackward isForwardOrBackwardOf(const Vector2D& from, const Vector2D& to) const
      {
        Vector2D difference = to - from;
        Vector2D tangentialAtFrom = tangential(from);
        return tangentialAtFrom.isForwardOrBackwardOf(difference);
      }

      /**
       *  Returns the end point which is first reached
       *  if one follows the forward direction of the circle starting from the start point.
       *  Evaluates which of the given end points end1 and end2 is closer to start
       *  This espicially treats the discontinuity on the far side of the circle correctly.
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

      /// Calculates the two points with the given cylindrical radius on the generalised circle
      std::pair<Belle2::TrackFindingCDC::Vector2D, Belle2::TrackFindingCDC::Vector2D>
      atCylindricalR(double cylindricalR) const;

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

      /**
       *  Approximate distance.
       *  Gives a fast signed approximation of the distance to the circle.
       *  The absolute value of the fast distance is accurate up to first
       *  order of distance/circle radius, hence best used with big circles and small distances.
       *  The sign of the fast distance indicates if the point is to the right or to the left of the
       *  circle.
       */
      double fastDistance(const Vector2D& point) const
      {
        return n0() + point.dot(n12()) + point.normSquared() * n3();
      }

      /// Approximate distance to the origin
      double fastImpact() const
      {
        return n0();
      }

      /**
       *  Gives the proper distance of the point to the circle line
       *  retaining the sign of the fast distance.
       */
      double distance(const Vector2D& point) const;

      /**
       *  Helper function to translate the fast linearized distance measure
       *  of the generalized circle to the proper distance from the circle
       *  retaining the sign of the distance.
       */
      double distance(double fastDistance) const;

      /**
       *  Helper function to translate the proper distance to the linearized distance measure of
       *  the generalized circle retaining the sign of the distance.
       */
      double fastDistance(const double distance) const
      {
        return (distance * n3() + 1.0) * distance;
      }

      /// Gives the signed distance of the origin to the circle
      double impact() const
      {
        return distance(fastImpact());
      }

      /// Getter for the absolute distance to the z axes at the support point
      double d0() const
      {
        return -impact();
      }

      /// Gives the tangential vector at the closest approach to the origin / at the perigee
      Vector2D tangential() const
      {
        return tangential(Vector2D(0.0, 0.0)).unit();
      }

      /// Gives to azimuth angle phi of the direction of flight at the perigee
      double tangentialPhi() const
      {
        return tangential().phi();
      }

      /// Gives the minimal cylindrical radius the circle reaches (unsigned)
      double minimalCylindricalR() const
      {
        return std::fabs(impact());
      }

      /// Gives the maximal cylindrical radius the circle reaches
      double maximalCylindricalR() const
      {
        return std::fabs(impact() + 2 * radius());
      }

      /// Gives the proper absolute distance of the point to the circle line.
      double absDistance(const Vector2D& point) const
      {
        return fabs(distance(point));
      }

      /**
       *  Indicates if the point is on the right or left side of the circle.
       *  This is also refered to as alpha.
       */
      ERightLeft isRightOrLeft(const Vector2D& point) const
      {
        return static_cast<ERightLeft>(sign(fastDistance(point)));
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

      /// Indicates if the generalized circle is actually a line
      bool isLine() const
      {
        return n3() == 0.0;
      }

      /// Indicates if the generalized circle is actually a circle
      bool isCircle() const
      {
        return n3() != 0.0;
      }

      /// Gives the signed radius of the circle. If it was a line this will be infinity
      double radius() const
      {
        return 1 / curvature();
      }

      /// Gives the signed radius of the circle. If it was a line this will be infinity
      double absRadius() const
      {
        return fabs(radius());
      }

      /// Gives the signed curvature of the generalized circle
      double curvature() const
      {
        return 2 * n3();
      }

      /**
       *  Gives the omega parameter as used by the framework helix.
       *  It is the signed curvature of the generalized circle with the opposite sign.
       */
      double omega() const
      {
        return -curvature();
      }

      /// Gives the center of the circle. If it was a line both components will be infinity
      Vector2D center() const
      {
        return n12().divided(-2 * n3());
      }

      /// Gives the perimeter of the circle.
      double perimeter() const
      {
        return 2 * M_PI * radius();
      }

      /**
       *  Gives the orientation of the circle.
       *  The circle can be either orientated counterclockwise or clockwise.
       *  @return ERotation::c_CounterClockwise for counterclockwise travel, ERotation::c_Clockwise
       * for clockwise.
       */
      ERotation orientation() const
      {
        return static_cast<ERotation>(sign(n3()));
      }

      /// Getter for the arc length for a full round of the circle
      double arcLengthPeriod() const
      {
        return std::fabs(perimeter());
      }

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

      /// Calculates the arc length between the perigee and the given point.
      double arcLengthTo(const Vector2D& to) const;

      /**
       *  Calculates the two dimensional arc length till the cylindrical radius is reached
       *  If the radius can not be reached return NAN.
       *  Note that there are two solutions which have equivalent arc lengths with different sign
       *  Always return the positive solution.
       */
      double arcLengthToCylindricalR(double cylindricalR) const;

      /**
       *  Helper function the calculate the factor between the length of a secant line and
       *  the length on the arc.
       *  Smooth function expressing the relation between arc length and direct length
       *  only using the curvature of the circle as additional information.
       *  It enables better handling of the line limit compared to the former implementaiton
       *  which used the opening angle of the arc.
       */
      double arcLengthFactor(const double directDistance) const
      {
        return arcLengthFactor(directDistance, curvature());
      }

      /**
       *  Helper function the calculate the factor between the length of a secant line and
       *  the length on the arc.
       *  Smooth function expressing the relation between arc length and direct length
       *  only using the curvature of the circle as additional information.
       *  It enables better handling of the line limit compared to the former implementaiton
       *  which used the opening angle of the arc.
       */
      static double arcLengthFactor(double directDistance, double curvature);

      /**
       *  Calculates the two points common to both circles.
       *  If the two points coincide both returned points are the same.
       *  If there is no common point both returned points will be made of NAN.
       */
      std::pair<Vector2D, Vector2D> intersections(const GeneralizedCircle& generalizedCircle) const;

      /**
       *  Calculates the point, which lies at the give perpendicular travel distance
       *  (counted from the perigee)
       */
      Vector2D atArcLength(double arcLength) const;

    private:
      // Order of this parameters make them easier to initialize

      /// Memory for the fourth parameter
      double m_n3;

      /// Memory for the second and third parameter
      Vector2D m_n12;

      /// Memory for the first parameter
      double m_n0;
    };

    /// Debug helper
    std::ostream& operator<<(std::ostream& output, const GeneralizedCircle& circle);
  }
}
