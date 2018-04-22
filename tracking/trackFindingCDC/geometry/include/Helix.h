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

#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/SZLine.h>

#include <tracking/trackFindingCDC/geometry/HelixParameters.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <iosfwd>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Extension of the generalized circle also caching the perigee coordinates.
    class Helix {

    public:
      /// Default constructor for ROOT compatibility.
      Helix()
        : m_circleXY()
        , m_szLine()
      {
      }

      /// Constructor combining a two dimensional circle with the linear augment in the sz space.
      Helix(const PerigeeCircle& circleXY, const SZLine& szLine)
        : m_circleXY(circleXY)
        , m_szLine(szLine)
      {
      }

      /// Constructor taking all stored parameters for internal use.
      explicit Helix(const HelixParameters& parameters)
        : m_circleXY(HelixUtil::getPerigeeParameters(parameters))
        , m_szLine(HelixUtil::getSZParameters(parameters))
      {
      }

      /// Constructor from all helix parameter
      Helix(double curvature, double phi0, double impact, double tanLambda, double z0)
        : m_circleXY(curvature, phi0, impact)
        , m_szLine(tanLambda, z0)
      {
      }

      /// Constructor from all helix parameter, phi given as a unit vector
      Helix(double curvature, const Vector2D& phi0Vec, double impact, double tanLambda, double z0)
        : m_circleXY(curvature, phi0Vec, impact)
        , m_szLine(tanLambda, z0)
      {
      }

      /// Sets all circle parameters to zero.
      void invalidate()
      {
        m_circleXY.invalidate();
        m_szLine.invalidate();
      }

      /// Indicates if the stored parameter combination designates a valid helix.
      bool isInvalid() const
      {
        return circleXY().isInvalid() and szLine().isInvalid();
      }

      /// Flips the travel direction of the helix in place, pivot point is unchanged.
      void reverse()
      {
        m_circleXY.reverse();
        m_szLine.reverse();
      }

      /// Returns a copy of the helix with flips the travel direction, pivot point is the same.
      Helix reversed() const
      {
        return Helix(circleXY().reversed(), szLine().reversed());
      }

    public:
      /// Calculates the perpendicular travel distance at which the helix has the closest approach
      /// to the given point.
      double arcLength2DToClosest(const Vector3D& point, bool firstPeriod = true) const;

      /**
       *  Calculates the two dimensional arc length that is closest to two dimensional point
       *  in the xy projection.
       *  Always gives a solution in the first half period in the positive or negative direction
       */
      double arcLength2DToXY(const Vector2D& point) const
      {
        return circleXY().arcLengthTo(point);
      }

      /**
       *  Calculates the two dimensional arc length that first reaches a cylindrical radius on the
       * helix
       *  Returns NAN if the radius cannot be reached.
       */
      double arcLength2DToCylindricalR(double cylindricalR) const
      {
        return circleXY().arcLengthToCylindricalR(cylindricalR);
      }

      /// Calculates the point on the helix with the smallest total distance
      Vector3D closest(const Vector3D& point, bool firstPeriod = true) const
      {
        double arcLength2D = arcLength2DToClosest(point, firstPeriod);
        return atArcLength2D(arcLength2D);
      }

      /// Calculates the point on the helix with the smallest perpendicular (xy) distance
      Vector3D closestXY(const Vector2D& pointXY) const
      {
        double arcLength2D = arcLength2DToXY(pointXY);
        return atArcLength2D(arcLength2D);
      }

      /// Calculates the distance of the point to the point of closest approach on the helix.
      double distance(const Vector3D& point) const
      {
        return point.distance(closest(point));
      }

      /// Calculates the distance of the line parallel to the z axes through the given point
      double distanceXY(const Vector2D& point) const
      {
        return m_circleXY.distance(point);
      }

      /**
       *  Moves the coordinates system by the given vector. Updates perigee point in place.
       *  @return arcLength2D that has to be traversed to the new origin
       */
      double passiveMoveBy(const Vector3D& by)
      {
        // First keep the necessary shift of the perpendicular travel distance to the new perigee
        // point.
        double byS = circleXY().arcLengthTo(by.xy());
        m_circleXY.passiveMoveBy(by.xy());
        Vector2D bySZ(byS, by.z());
        m_szLine.passiveMoveBy(bySZ);
        return byS;
      }

      /// Computes the Jacobi matrix for a move of the coordinate system by the given vector.
      HelixJacobian passiveMoveByJacobian(const Vector3D& by) const;

      /**
       *  Adjust the arclength measure to start n periods later.
       *  @return The arc length needed to travel n periods.
       */
      double shiftPeriod(int nPeriods)
      {
        double arcLength2D = nPeriods * fabs(perimeterXY());
        m_szLine.passiveMoveBy(Vector2D(arcLength2D, 0.0));
        return arcLength2D;
      }

      /// Calculates the point, which lies at the give perpendicular travel distance (counted from
      /// the perigee)
      Vector3D atArcLength2D(double s) const
      {
        return Vector3D(circleXY().atArcLength(s), szLine().map(s));
      }

      /// Calculates the point, which lies at the given z coordinate
      Vector3D atZ(double z) const
      {
        return Vector3D(xyAtZ(z), z);
      }

      /// Calculates the point, which lies at the given z coordinate
      Vector2D xyAtZ(double z) const
      {
        return Vector2D(circleXY().atArcLength(szLine().inverseMap(z)));
      }

      /// Gives the minimal cylindrical radius the circle reaches (unsigned)
      double minimalCylindricalR() const
      {
        return circleXY().minimalCylindricalR();
      }

      /// Gives the maximal cylindrical radius the circle reaches
      double maximalCylindricalR() const
      {
        return circleXY().maximalCylindricalR();
      }

      /// Getter for the signed curvature in the xy projection.
      double curvatureXY() const
      {
        return circleXY().curvature();
      }

      /// Getter for the signed distance to the z axes at the perigee point.
      double impactXY() const
      {
        return circleXY().impact();
      }

      /// Getter for the omega parameter of the common helix parameterisation.
      double omega() const
      {
        return circleXY().omega();
      }

      /// Getter for the signed distance to the z axes at the perigee point
      double d0() const
      {
        return circleXY().d0();
      }

      /// Getter for the perigee point in the xy projection.
      Vector2D perigeeXY() const
      {
        return circleXY().perigee();
      }

      /// Getter for the perigee point of the helix.
      Vector3D perigee() const
      {
        return Vector3D(perigeeXY(), z0());
      }

      /// Getter for the proportinality factor from arc length in xy space to z.
      double tanLambda() const
      {
        return m_szLine.tanLambda();
      }

      /// Getter for the proportinality factor from arc length in xy space to z.
      double cotTheta() const
      {
        return tanLambda();
      }

      /// Getter for z coordinate at the perigee point of the helix.
      double z0() const
      {
        return m_szLine.z0();
      }

      /// Getter for the distance in z at which the two points on the helix coincide in the xy
      /// projection
      double zPeriod() const
      {
        return tanLambda() * arcLength2DPeriod();
      }

      /// Getter for the arc length of one trip around the helix.
      double arcLength2DPeriod() const
      {
        return circleXY().arcLengthPeriod();
      }

      /// Getter for the perimeter of the circle in the xy projection
      double perimeterXY() const
      {
        return circleXY().perimeter();
      }

      /// Getter for the radius of the circle in the xy projection
      double radiusXY() const
      {
        return circleXY().radius();
      }

      /// Getter for the central point of the helix
      Vector2D centerXY() const
      {
        return circleXY().center();
      }

      /// Getter for the unit three dimensional tangential vector at the perigee point of the helix.
      Vector3D tangential() const
      {
        return Vector3D(phi0Vec(), tanLambda()).unit();
      }

      /// Getter for the direction vector in the xy projection at the perigee of the helix.
      const Vector2D& phi0Vec() const
      {
        return circleXY().phi0Vec();
      }

      /// Getter for the azimuth angle of the direction of flight at the perigee.
      double phi0() const
      {
        return circleXY().phi0();
      }

      /// Getter for the five helix parameters in the order defined by EHelixParameter.h
      HelixParameters helixParameters() const
      {
        HelixParameters result;
        using namespace NHelixParameterIndices;
        result(c_Curv) = curvatureXY();
        result(c_Phi0) = phi0();
        result(c_I) = impactXY();
        result(c_TanL) = tanLambda();
        result(c_Z0) = z0();
        return result;
      }

      /// Getter for the projection into xy space
      const PerigeeCircle& circleXY() const
      {
        return m_circleXY;
      }

      /// Getter for the projection into xy space
      const SZLine& szLine() const
      {
        return m_szLine;
      }

    private:
      /// Memory of the projection of the helix in xy space.
      PerigeeCircle m_circleXY;

      /// Memory of the of the linear relation between perpendicular travel distance and the z
      /// position.
      SZLine m_szLine;

    };

    /// Debug helper
    std::ostream& operator<<(std::ostream& output, const Helix& helix);
  }
}
