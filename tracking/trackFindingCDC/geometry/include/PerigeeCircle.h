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
#include <tracking/trackFindingCDC/geometry/Circle2D.h>
#include <tracking/trackFindingCDC/geometry/Line2D.h>

#include <tracking/trackFindingCDC/geometry/PerigeeParameters.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/numerics/Angle.h>

#include <TVectorD.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Extension of the generalized circle also caching the perigee coordinates.
    class PerigeeCircle : public GeneralizedCircle {

    public:
      /// Default constructor for ROOT compatibility.
      PerigeeCircle();

      /// Constructor from the perigee parammeters. The direction of travel at the perigee is given as vector.
      PerigeeCircle(double curvature, const Vector2D& phi0Vec, double impact);

      /// Constructor from the perigee parammeters. The direction of travel at the perigee is given as azimuth angle
      PerigeeCircle(double curvature, double phi0, double impact);

      /// Constructor from the perigee parammeters.
      explicit PerigeeCircle(const PerigeeParameters& par);

    private:
      /// Constructor taking all stored parameters for internal use.
      PerigeeCircle(const GeneralizedCircle& n0123,
                    double curvature,
                    double phi0,
                    const Vector2D& phi0Vec,
                    double impact);

    public:
      /// Constructor from a two dimensional line
      explicit PerigeeCircle(const Line2D& n012);

      /// Constructor pomoting the generalized circle
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
      static PerigeeCircle fromCenterAndRadius(const Vector2D& center,
                                               double absRadius,
                                               const ERotation orientation = ERotation::c_CounterClockwise);


    protected:
      /// Caches the perigee parameters from the underlying generalized circle parameters.
      void receivePerigeeParameters();

    public:
      /// Sets all circle parameters to zero
      void invalidate();

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

      /// Puts the Jacobi matrix for a move of the coordinate system by the given vector in the given matrix as an output argument
      void passiveMoveByJacobian(const Vector2D& by, PerigeeJacobian& jacobian) const;

      /// Calculates the point, which lies at the give perpendicular travel distance (counted from the perigee)
      Vector2D atArcLength(double arcLength) const;

      /// Calculates the arc length between the perigee and the given point.
      double arcLengthTo(const Vector2D& point) const;

      /**
       *  Calculates the two dimensional arc length till the cylindrical radius is reached
       *  If the radius can not be reached return NAN.
       *  Note that there are two solutions which have equivalent arc lengths with different sign
       *  Always return the positive solution.
       */
      double arcLengthToCylindricalR(double cylindricalR) const;

      /// Calculates the point of closest approach on the circle to the given point.
      Vector2D closest(const Vector2D& point) const;

      /// Getter for the proper signed distance of the point to the circle
      double distance(const Vector2D& point) const
      { return distance(fastDistance(point)); }

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
      { return fastDistance(impact()); }

      /**
       *  Helper function to translate the proper distance to the linearized distance measure of
       *  the circle retaining the sign of the distance.
       */
      double fastDistance(double distance) const
      { return distance * (1.0 + distance * curvature() / 2); }

      /// Indicates if the point is on the right or left side of the circle.
      ERightLeft isRightOrLeft(const Vector2D& point) const
      { return static_cast<ERightLeft>(sign(fastDistance(point))); }

      /// Indicates if the perigee parameters represent a line
      bool isLine() const
      { return curvature() == 0.0; }

      /// Indicates if the perigee parameters represent a closed circle
      bool isCircle() const
      { return curvature() != 0.0; }

      /**
       *  Gives the orientation of the circle.
       *  The circle can be either orientated counterclockwise or clockwise.
       *  @return ERotation::c_CounterClockwise for counterclockwise travel, ERotation::c_Clockwise for clockwise.
       */
      inline ERotation orientation() const
      { return static_cast<ERotation>(sign(curvature())); }

      /// Getter for the tangtial vector at the perigee
      Vector2D tangential(const Vector2D& pos) const
      { return GeneralizedCircle::tangential(pos); }

      /// Getter for the tangtial vector at the perigee
      const Vector2D& tangential() const
      { return phi0Vec(); }

      /// Getter for the azimuth angle of the direction of flight at the perigee - Deprecated for phi0()
      double tangentialPhi() const
      { return phi0(); }

      /// Getter for the perigee point
      Vector2D perigee() const
      { return phi0Vec().orthogonal() * impact(); }

      /// Getter for the center of the circle. If it was a line both components will be infinity.
      Vector2D center() const
      { return phi0Vec().orthogonal() * (impact() + radius()); }

      /// Getter for the apogee of the circle. If it was a line both components will be infinity.
      Vector2D apogee() const
      { return phi0Vec().orthogonal() * (impact() + 2 * radius()); }

      /// Gives the minimal cylindrical radius the circle reaches (unsigned)
      double minimalCylindricalR() const
      { return fabs(impact()); }

      /// Gives the maximal cylindrical radius the circle reaches
      double maximalCylindricalR() const
      { return fabs(impact() + 2 * radius()); }

      /// Getter for the arc length for a full round of the circle
      double arcLengthPeriod() const
      { return std::fabs(perimeter()); }

      /// Gives the signed perimeter of the circle.
      double perimeter() const
      { return 2 * M_PI * radius(); }

      /// Gives the signed radius of the circle. If it was a line this will be infinity.
      double radius() const
      { return 1 / curvature(); }

      /// Gives the signed radius of the circle. If it was a line this will be infinity.
      double absRadius() const
      { return fabs(radius()); }

      /// Setter for the circle center and radius
      void setCenterAndRadius(const Vector2D& center,
                              double absRadius,
                              const ERotation orientation = ERotation::c_CounterClockwise);

      /// Getter for the generalised circle parameter n0
      double n0() const
      { return impact() * (impact() * curvature() / 2.0 + 1.0); }

      /// Getter for the generalised circle parameters n1 and n2
      Vector2D n12() const
      { return -phi0Vec().orthogonal() * (1 + curvature() * impact()); }

      /// Getter for the generalised circle parameter n0
      double n3() const
      { return curvature() / 2.0; }

      /// Setter for four generalised circle parameters.
      void setN(double n0, double n1, double n2, double n3 = 0.0)
      { setN(n0, Vector2D(n1, n2), n3); }

      /// Setter for four generalised circle parameters.
      void setN(double n0, const Vector2D& n12, double n3 = 0.0);

      /// Setter for generalised circle parameters from a normal line.
      void setN(const Line2D& n012)
      { setN(n012.n0(), n012.n12()); }

      /// Setter for four generalised circle parameters.
      void setN(const GeneralizedCircle& n0123)
      { setN(n0123.n0(), n0123.n12(), n0123.n3()); }

      /// Getter for omega parameter of the common Belle2::Helix which is the wrong sign curvature
      double omega() const
      { return -curvature(); }

      /// Getter for d0 parameter of the common Belle2::Helix representation
      double d0() const
      { return -impact(); }

      /// Getter for the signed curvature.
      double curvature() const
      { return m_curvature; }

      /// Getter for the azimuth angle of the direction of flight at the perigee
      double phi0() const
      { return m_phi0; }

      /// Getter for the unit vector of the direction of flight at the perigee
      const Vector2D& phi0Vec() const
      { return m_phi0Vec; }

      /// Getter for the signed distance of the origin to the circle.
      double impact() const
      { return m_impact; }

      /// Getter for the three perigee parameters in the order defined by EPerigeeParameter.h
      PerigeeParameters parameters() const
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
        GeneralizedCircle::setPerigeeParameters(m_curvature, m_phi0Vec, m_impact);
      }

      /// Sets the azimuth angle of the direction of flight at the perigee.
      void setTangentialPhi(double phi0)
      {
        m_phi0 = phi0;
        m_phi0Vec = Vector2D::Phi(phi0);
        GeneralizedCircle::setPerigeeParameters(m_curvature, m_phi0Vec, m_impact);
      }

      /// Sets the unit direction of flight at the perigee
      void setTangential(const Vector2D& tangential)
      {
        m_phi0 = tangential.phi();
        m_phi0Vec = tangential.unit();
        GeneralizedCircle::setPerigeeParameters(m_curvature, m_phi0Vec, m_impact);
      }

      /// Sets the impact parameter of the circle.
      void setImpact(double impact)
      {
        m_impact = impact;
        GeneralizedCircle::setPerigeeParameters(m_curvature, m_phi0Vec, m_impact);
      }

      /// Setter for the perigee parameters
      void setPerigeeParameters(double curvature, const Vector2D& phi0Vec, double impact)
      {
        m_impact = impact;
        m_phi0 = phi0Vec.phi();
        m_phi0Vec = phi0Vec;
        m_curvature = curvature;
        GeneralizedCircle::setPerigeeParameters(m_curvature, m_phi0Vec, m_impact);
      }

      /// Setter for the perigee parameters
      void setPerigeeParameters(double curvature, double phi0, double impact)
      {
        m_impact = impact;
        m_phi0 = phi0;
        m_phi0Vec = Vector2D::Phi(phi0);
        m_curvature = curvature;
        GeneralizedCircle::setPerigeeParameters(m_curvature, m_phi0Vec, m_impact);
      }

      /// Debug helper
      friend std::ostream& operator<<(std::ostream& output, const PerigeeCircle& circle)
      {
        return output <<
               "PerigeeCircle(" <<
               "curvature=" << circle.curvature() << "," <<
               "phi0=" << circle.phi0() << "," <<
               "impact=" << circle.impact() << ")" ;
      }

    private:

      double arcLengthAtDeltaLength(double delta, double dr) const;
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

    }; //class


  } // namespace TrackFindingCDC
} // namespace Belle2
