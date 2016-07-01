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
      PerigeeCircle(const double curvature,
                    const Vector2D& tangential,
                    const double impact);

      /// Constructor from the perigee parammeters. The direction of travel at the perigee is given as azimuth angle
      PerigeeCircle(const double curvature,
                    const double tangentialPhi,
                    const double impact);

      /// Constructor from the perigee parammeters.
      explicit PerigeeCircle(const PerigeeParameters& par);


    private:
      /// Constructor taking all stored parameters for internal use.
      PerigeeCircle(const GeneralizedCircle& n0123,
                    const double curvature,
                    const double tangentialPhi,
                    const Vector2D& tangential,
                    const double impact);

    public:
      /// Constructor from a two dimensional line
      explicit PerigeeCircle(const Line2D& n012);

      /// Constructor pomoting the generalized circle
      explicit PerigeeCircle(const GeneralizedCircle& n0123);

      /// Constructor from a two dimensional circle in center / radius representation
      explicit PerigeeCircle(const Circle2D& circle);

      /// Constructor with the four parameters of the generalized circle
      static PerigeeCircle fromN(const double n0,
                                 const double n1,
                                 const double n2,
                                 const double n3 = 0);

      /// Constructor with the four parameters of the generalized circle
      static PerigeeCircle fromN(const double n0,
                                 const Vector2D& n12,
                                 const double n3 = 0);


      /// Constructor from center, radius and a optional orientation
      /** The center and radius alone do not carry any orientation. However the generalized circle does.
       *  This constructor makes an orientated representation from them. If not given the orientation defaults to
       *  mathematical positiv counterclockwise. */
      static PerigeeCircle fromCenterAndRadius(const Vector2D& center,
                                               const double absRadius,
                                               const ERotation orientation = ERotation::c_CounterClockwise);





    protected:
      /// Caches the perigee parameters from the underlying generalized circle parameters.
      void receivePerigeeParameters()
      {
        m_impact = GeneralizedCircle::impact();
        m_curvature = GeneralizedCircle::curvature();
        m_tangential = GeneralizedCircle::tangential();
        m_tangentialPhi = GeneralizedCircle::tangentialPhi();
        m_impact = GeneralizedCircle::impact();
      }

    public:
      /// Setter for the circle center and radius
      void setCenterAndRadius(const Vector2D& center,
                              const double absRadius,
                              const ERotation orientation = ERotation::c_CounterClockwise)
      {
        GeneralizedCircle::setCenterAndRadius(center, absRadius, orientation);
        receivePerigeeParameters();
      }

      /// Setter for the perigee parameters
      void setPerigeeParameters(const double curvature,
                                const Vector2D& tangential,
                                const double impact)
      {
        m_impact = impact;
        m_tangentialPhi = tangential.phi();
        m_tangential = tangential;
        m_curvature = curvature;
        GeneralizedCircle::setPerigeeParameters(m_curvature, m_tangential, m_impact);
      }


      /// Setter for the perigee parameters
      inline void setPerigeeParameters(const double curvature,
                                       const double tangentialPhi,
                                       const double impact)
      {
        m_impact = impact;
        m_tangentialPhi = tangentialPhi;
        m_tangential = Vector2D::Phi(tangentialPhi);
        m_curvature = curvature;
        GeneralizedCircle::setPerigeeParameters(m_curvature, m_tangential, m_impact);
      }

      /// Setter for all four circle parameters.
      /// The normal representation of a line leave out the last parameter
      void setN(const double n0, const double n1, const double n2, const double n3 = 0.0)
      {
        GeneralizedCircle::setN(n0, n1, n2, n3);
        receivePerigeeParameters();
      }

      /// Setter for all four circle parameters.
      /// The normal representation of a line leave out the last parameter
      void setN(const double n0, const Vector2D& n12, const double n3 = 0.0)
      {
        GeneralizedCircle::setN(n0, n12, n3);
        receivePerigeeParameters();
      }

      /// Setter for all four circle parameters from a line.
      void setN(const Line2D& n012)
      {
        GeneralizedCircle::setN(n012);
        receivePerigeeParameters();
      }

      /// Setter for all four circle parameters from another circle
      void setN(const GeneralizedCircle& n0123)
      {
        GeneralizedCircle::setN(n0123);
        receivePerigeeParameters();
      }

      /// Setter for signed curvature.
      inline void setCurvature(const double curvature)
      {
        m_curvature = curvature;
        GeneralizedCircle::setPerigeeParameters(m_curvature, m_tangential, m_impact);
      }

      /// Sets the azimuth angle of the direction of flight at the perigee.
      inline void setTangentialPhi(const double tangentialPhi)
      {
        m_tangentialPhi = tangentialPhi;
        m_tangential = Vector2D::Phi(tangentialPhi);
        GeneralizedCircle::setPerigeeParameters(m_curvature, m_tangential, m_impact);
      }

      /// Sets the unit direction of flight at the perigee
      inline void setTangential(const Vector2D& tangential)
      {
        m_tangentialPhi = tangential.phi();
        m_tangential = tangential.unit();
        GeneralizedCircle::setPerigeeParameters(m_curvature, m_tangential, m_impact);
      }

      /// Sets the impact parameter of the circle.
      inline void setImpact(const double impact)
      {
        m_impact = impact;
        GeneralizedCircle::setPerigeeParameters(m_curvature, m_tangential, m_impact);
      }

      /// Sets all circle parameters to zero
      void invalidate()
      {
        GeneralizedCircle::invalidate();
        m_curvature = 0.0;
        m_tangentialPhi = NAN;
        m_tangential = Vector2D(0.0, 0.0);
        m_impact = 0;
      }

      /// Flips the orientation of the circle in place
      inline void reverse()
      {
        GeneralizedCircle::reverse();
        m_curvature = -m_curvature;
        m_tangentialPhi = AngleUtil::reversed(m_tangentialPhi);
        m_tangential.reverse();
        m_impact = -m_impact;
      }

      /// Returns a copy of the circle with opposite orientation.
      inline PerigeeCircle reversed() const
      {
        return PerigeeCircle(GeneralizedCircle::reversed(),
                             -m_curvature,
                             AngleUtil::reversed(m_tangentialPhi),
                             -m_tangential,
                             -m_impact
                            );
      }

    public:
      /// Transforms the generalized circle to conformal space inplace
      /** Applies the conformal map in the self-inverse from  X = x / (x^2 + y^2) and Y = y / (x^2 +y^2) inplace
       *  It works most easily by the exchange of the circle parameters n0 <-> n3 */
      inline void conformalTransform()
      {
        GeneralizedCircle::conformalTransform();
        receivePerigeeParameters();
      }

      /// Returns a copy of the circle in conformal space
      /** Applies the conformal map in the self-inverse from  X = x / (x^2 + y^2) and Y = y / (x^2 +y^2) and returns the result as a new GeneralizedCircle
       *  It works most easily by the exchange of the circle parameters n0 <-> n3 */
      inline PerigeeCircle conformalTransformed() const
      {
        return PerigeeCircle::fromN(n3(), n12(), n0());
      }


      /// Moves the coordinates system by the given vector. Updates perigee parameters in place
      void passiveMoveBy(const Vector2D& by)
      {
        GeneralizedCircle::passiveMoveBy(by);
        receivePerigeeParameters();
      }
      /// Computes the Jacobi matrix for a move of the coordinate system by the given vector.
      PerigeeJacobian passiveMoveByJacobian(const Vector2D& by) const;

      /// Puts the Jacobi matrix for a move of the coordinate system by the given vector in the given matrix as an output argument
      void passiveMoveByJacobian(const Vector2D& by, PerigeeJacobian& jacobian) const;

      /// Calculates the point, which lies at the give perpendicular travel distance (counted from the perigee)
      Vector2D atArcLength(const double arcLength) const;

      /** Calculates the two dimensional arc length till the cylindrical radius is reached
       *  If the radius can not be reached return NAN.
       *  Note that there are two solutions which have equivalent arc lengths with different sign
       *  Always return the positive solution. */
      double arcLengthToCylindricalR(const double cylindricalR) const;

      ///Getter for the signed curvature.
      inline double curvature() const
      { return m_curvature; }

      /// Gives the signed distance of the origin to the circle.
      inline double impact() const
      { return m_impact; }

      /// Getter for the absolute distance to the z axes at the support point
      double d0() const
      { return -impact(); }

      /// Gets the azimuth angle of the direction of flight at the perigee
      inline double tangentialPhi() const
      { return m_tangentialPhi; }

      /// Getter for the tangtial vector at the perigee
      inline const Vector2D& tangential() const
      { return m_tangential; }

      /// Gets the azimuth angle of the direction of flight at the perigee
      inline double phi0() const
      { return tangentialPhi(); }

      /// Getter for the tangtial vector at the perigee
      inline Vector2D tangential(const Vector2D& pos) const
      { return GeneralizedCircle::tangential(pos); }

      /// Getter for the perigee point
      inline Vector2D perigee() const
      { return tangential().orthogonal() * impact(); }

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

      /// Gives the minimal cylindrical radius the circle reaches (unsigned)
      inline double minimalCylindricalR() const
      { return fabs(impact()); }

      /// Gives the maximal cylindrical radius the circle reaches
      inline double maximalCylindricalR() const
      { return fabs(impact() + 2 * radius()); }


      /// Debug helper
      friend std::ostream& operator<<(std::ostream& output, const PerigeeCircle& circle)
      {
        return output <<
               "PerigeeCircle(" <<
               "curvature=" << circle.curvature() << "," <<
               "tangentialPhi=" << circle.tangentialPhi() << "," <<
               "impact=" << circle.impact() << ")" ;
      }



    private:
      /// Memory for the signed curvature
      double m_curvature;

      /// Memory for the azimuth angle of the direction of flight at the perigee
      double m_tangentialPhi;

      /// Cached unit direction of flight at the perigee
      Vector2D m_tangential;

      /// Memory for the signed impact parameter
      double m_impact;

    }; //class


  } // namespace TrackFindingCDC
} // namespace Belle2
