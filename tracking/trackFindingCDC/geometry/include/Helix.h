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

#include <cmath>

#include <TVectorD.h>


#include <tracking/trackFindingCDC/geometry/EHelixParameter.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Line2D.h>
#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Extension of the generalized circle also caching the perigee coordinates.
    class Helix  {

    public:

      /// Default constructor for ROOT compatibility.
      Helix() :
        m_circleXY(),
        m_lineSZ()
      {}

      /// Constructor combining a two dimensional circle with the linear augment in the sz space.
      Helix(const PerigeeCircle& circleXY,
            const Line2D& lineSZ) :
        m_circleXY(circleXY),
        m_lineSZ(lineSZ)
      {}

      /// Constructor taking all stored parameters for internal use.
      explicit Helix(const TVectorD& parameters) :
        m_circleXY(parameters(EHelixParameter::c_Curv),
                   parameters(EHelixParameter::c_Phi0),
                   parameters(EHelixParameter::c_I)),
        m_lineSZ(Line2D::fromSlopeIntercept(parameters(EHelixParameter::c_TanL),
                                            parameters(EHelixParameter::c_Z0)))
      {}

      /// Constructor from all helix parameter
      Helix(const double curvature,
            const double tangentialPhi,
            const double impact,
            const double tanLambda,
            const double z0) :
        m_circleXY(curvature, tangentialPhi, impact),
        m_lineSZ(Line2D::fromSlopeIntercept(tanLambda, z0))
      {}

      /// Constructor from all helix parameter, phi given as a unit vector
      Helix(const double curvature,
            const Vector2D& tangential,
            const double impact,
            const double tanLambda,
            const double z0) :
        m_circleXY(curvature, tangential, impact),
        m_lineSZ(Line2D::fromSlopeIntercept(tanLambda, z0))
      {}

      /// Sets all circle parameters to zero.
      void invalidate()
      {
        m_circleXY.invalidate();
        m_lineSZ.invalidate();
      }

      /// Indicates if the stored parameter combination designates a valid helix.
      bool isInvalid() const
      { return circleXY().isInvalid() and lineSZ().isInvalid(); }

      /// Flips the travel direction of the helix in place, pivot point is unchanged.
      inline void reverse()
      {
        m_circleXY.reverse();
        //Invert the travel distance scale, but not the z scale.
        m_lineSZ.flipFirst();
      }

      /// Returns a copy of the helix with flips the travel direction, pivot point is the same.
      inline Helix reversed() const
      { return Helix(circleXY().reversed(), lineSZ().flippedFirst()); }

    public:
      /// Calculates the perpendicular travel distance at which the helix has the closest approach to the given point.
      double arcLength2DToClosest(const Vector3D& point) const;

      /** Calculates the two dimensional arc length that is closest to two dimensional point
       *  in the xy projection.
       *  Always gives a solution in the first half period in the positive or negative direction
       */
      double arcLength2DToXY(const Vector2D& point) const
      { return circleXY().arcLengthTo(point); }

      /** Calculates the two dimensional arc length that first reaches a cylindrical radius on the helix
       *  Returns NAN if the radius cannot be reached.*/
      double arcLength2DToCylindricalR(const double cylindricalR) const
      { return circleXY().arcLengthToCylindricalR(cylindricalR); }

      /// Calculates the point of closest approach on the helix to the given point.
      Vector3D closest(const Vector3D& point) const
      {
        double arcLength2D = arcLength2DToClosest(point);
        return atArcLength2D(arcLength2D);
      }

      /// Calculates the distance of the point to the point of closest approach on the helix.
      double distance(const Vector3D& point) const
      { return point.distance(closest(point));}

      //double lengthOnCurve(const Vector3D& from, const Vector3D& to) const;

      /** Moves the coordinates system by the given vector. Updates support point in place.
       *  @return arcLength2D that has to be traversed to the new origin
       */
      double passiveMoveBy(const Vector3D& by)
      {
        // First keep the necessary shift of the perpendicular travel distance to the new support point.
        double byS = circleXY().arcLengthTo(by.xy());
        m_circleXY.passiveMoveBy(by.xy());
        Vector2D bySZ(byS, by.z());
        m_lineSZ.passiveMoveBy(bySZ);
        return byS;
      }


      /// Computes the Jacobi matrix for a move of the coordinate system by the given vector.
      TMatrixD passiveMoveByJacobian(const Vector3D& by) const;

      /// Shifts the tanLambda and z0 by the given amount. Method is specific to the corrections in the fusion fit.
      void shiftTanLambdaZ0(const double tanLambdaShift, const double zShift)
      {
        double z0 = m_lineSZ.intercept();
        double tanLambda = m_lineSZ.slope();
        m_lineSZ.setSlopeIntercept(tanLambda + tanLambdaShift, z0 + zShift);
      }

      /// Calculates the point, which lies at the give perpendicular travel distance (counted from the perigee)
      Vector3D atArcLength2D(const double s) const
      { return Vector3D(circleXY().atArcLength(s), lineSZ().map(s)); }

      /// Calculates the point, which lies at the given z coordinate
      Vector3D atZ(const double z) const
      { return Vector3D(xyAtZ(z), z); }

      /// Calculates the point, which lies at the given z coordinate
      Vector2D xyAtZ(const double z) const
      { return Vector2D(circleXY().atArcLength(lineSZ().inverseMap(z))); }


      /// Gives the minimal cylindrical radius the circle reaches (unsigned)
      inline double minimalCylindricalR() const
      { return circleXY().minimalCylindricalR(); }

      /// Gives the maximal cylindrical radius the circle reaches
      inline double maximalCylindricalR() const
      { return circleXY().maximalCylindricalR(); }

      /// Getter for the signed curvature in the xy projection.
      inline double curvatureXY() const
      { return circleXY().curvature(); }

      /// Getter for the signed distance to the z axes at the support point.
      double impactXY() const
      { return circleXY().impact(); }

      /// Getter for the signed distance to the z axes at the support point
      double d0() const
      { return circleXY().d0(); }

      /// Getter for the perigee point in the xy projection.
      Vector2D perigeeXY() const
      { return circleXY().perigee(); }

      /// Getter for the support point of the helix.
      /** The support point marks the zero of the travel distance on the helix curve.
       *  It can not be choosen abitrary but has to be equivalent to the perigee point in the xy projection.
       */
      Vector3D support() const
      { return Vector3D(perigeeXY(), z0()); }

      /// Getter for the proportinality factor from arc length in xy space to z.
      double tanLambda() const
      { return m_lineSZ.slope(); }

      /// Getter for the proportinality factor from arc length in xy space to z.
      double cotTheta() const
      { return tanLambda(); }

      /// Getter for z coordinate at the support point of the helix.
      double z0() const
      { return m_lineSZ.intercept(); }

      /// Getter for the distance in z at which the two points on the helix coincide in the xy projection
      double zPeriod() const
      { return lineSZ().map(perimeterXY()); }

      /// Getter for the perimeter of the circle in the xy projection
      double perimeterXY() const
      { return circleXY().perimeter(); }

      /// Getter for the radius of the circle in the xy projection
      double radiusXY() const
      { return circleXY().radius(); }

      /// Getter for the unit three dimensional tangential vector at the support point of the helix.
      Vector3D tangential() const
      {
        Vector3D result(tangentialXY(), tanLambda());
        result.normalize();
        return result;
      }

      /// Getter for the tangential vector in the xy projection at the support point of the helix.
      const Vector2D& tangentialXY() const
      { return circleXY().tangential(); }

      /// Getter for the azimuth angle of the tangential vector at the support point of the helix.
      double tangentialPhi() const
      { return circleXY().tangentialPhi(); }

      /// Getter for the azimuth angle of the tangential vector at the support point of the helix.
      double phi0() const
      { return tangentialPhi(); }

      /// Getter for the five helix parameters in the order defined by EHelixParameter.h and EPerigeeParameter.h
      TVectorD parameters() const
      {
        using namespace NHelixParameter;
        TVectorD result(c_Curv, c_Z0);
        result(c_Curv) = curvatureXY();
        result(c_Phi0) = phi0();
        result(c_I) = impactXY();
        result(c_TanL) = tanLambda();
        result(c_Z0) = z0();
        return result;
      }



      /// Getter for the projection into xy space
      const PerigeeCircle& circleXY() const
      { return m_circleXY; }

      /// Getter for the projection into xy space
      const Line2D& lineSZ() const
      { return m_lineSZ; }

    private:
      /// Memory of the projection of the helix in xy space.
      PerigeeCircle m_circleXY;

      /// Memory of the of the linear relation between perpendicular travel distance and the z position.
      Line2D m_lineSZ;


    }; //class


  } // namespace TrackFindingCDC
} // namespace Belle2
