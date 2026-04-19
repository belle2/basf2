/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/geometry/PerigeeCircle.h>
#include <tracking/trackingUtilities/geometry/SZLine.h>

#include <tracking/trackingUtilities/geometry/HelixParameters.h>
#include <tracking/trackingUtilities/geometry/VectorUtil.h>

#include <Math/Vector3D.h>
#include <Math/Vector2D.h>

#include <iosfwd>
#include <cmath>

namespace Belle2 {
  namespace TrackingUtilities {

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
      Helix(double curvature, const ROOT::Math::XYVector& phi0Vec, double impact, double tanLambda, double z0)
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
      double arcLength2DToClosest(const ROOT::Math::XYZVector& point, bool firstPeriod = true) const;

      /**
       *  Calculates the two dimensional arc length that is closest to two dimensional point
       *  in the xy projection.
       *  Always gives a solution in the first half period in the positive or negative direction
       */
      double arcLength2DToXY(const ROOT::Math::XYVector& point) const
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
      ROOT::Math::XYZVector closest(const ROOT::Math::XYZVector& point, bool firstPeriod = true) const
      {
        double arcLength2D = arcLength2DToClosest(point, firstPeriod);
        return atArcLength2D(arcLength2D);
      }

      /// Calculates the point on the helix with the smallest perpendicular (xy) distance
      ROOT::Math::XYZVector closestXY(const ROOT::Math::XYVector& pointXY) const
      {
        double arcLength2D = arcLength2DToXY(pointXY);
        return atArcLength2D(arcLength2D);
      }

      /// Calculates the distance of the point to the point of closest approach on the helix.
      double distance(const ROOT::Math::XYZVector& point) const
      {
        return VectorUtil::Distance(point, closest(point));
      }

      /// Calculates the distance of the line parallel to the z axes through the given point
      double distanceXY(const ROOT::Math::XYVector& point) const
      {
        return m_circleXY.distance(point);
      }

      /**
       *  Moves the coordinates system by the given vector. Updates perigee point in place.
       *  @return arcLength2D that has to be traversed to the new origin
       */
      double passiveMoveBy(const ROOT::Math::XYZVector& by)
      {
        // First keep the necessary shift of the perpendicular travel distance to the new perigee
        // point.
        double byS = circleXY().arcLengthTo(VectorUtil::get2DVector(by));
        m_circleXY.passiveMoveBy(VectorUtil::get2DVector(by));
        ROOT::Math::XYVector bySZ(byS, by.z());
        m_szLine.passiveMoveBy(bySZ);
        return byS;
      }

      /// Computes the Jacobi matrix for a move of the coordinate system by the given vector.
      HelixJacobian passiveMoveByJacobian(const ROOT::Math::XYZVector& by) const;

      /**
       *  Adjust the arclength measure to start n periods later.
       *  @return The arc length needed to travel n periods.
       */
      double shiftPeriod(int nPeriods)
      {
        double arcLength2D = nPeriods * fabs(perimeterXY());
        m_szLine.passiveMoveBy(ROOT::Math::XYVector(arcLength2D, 0.0));
        return arcLength2D;
      }

      /// Calculates the point, which lies at the give perpendicular travel distance (counted from
      /// the perigee)
      ROOT::Math::XYZVector atArcLength2D(double s) const
      {
        const auto& tmp = circleXY().atArcLength(s);
        return ROOT::Math::XYZVector(tmp.X(), tmp.Y(), szLine().map(s));
      }

      /// Calculates the point, which lies at the given z coordinate
      ROOT::Math::XYZVector atZ(double z) const
      {
        const auto& tmp = xyAtZ(z);
        return ROOT::Math::XYZVector(tmp.X(), tmp.Y(), z);
      }

      /// Calculates the point, which lies at the given z coordinate
      ROOT::Math::XYVector xyAtZ(double z) const
      {
        return ROOT::Math::XYVector(circleXY().atArcLength(szLine().inverseMap(z)));
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
      ROOT::Math::XYVector perigeeXY() const
      {
        return circleXY().perigee();
      }

      /// Getter for the perigee point of the helix.
      ROOT::Math::XYZVector perigee() const
      {
        const auto& tmp = perigeeXY();
        return ROOT::Math::XYZVector(tmp.X(), tmp.Y(), z0());
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
      ROOT::Math::XYVector centerXY() const
      {
        return circleXY().center();
      }

      /// Getter for the unit three dimensional tangential vector at the perigee point of the helix.
      ROOT::Math::XYZVector tangential() const
      {
        const auto& tmp = phi0Vec();
        return VectorUtil::unit(ROOT::Math::XYZVector(tmp.X(), tmp.Y(), tanLambda()));
      }

      /// Getter for the direction vector in the xy projection at the perigee of the helix.
      const ROOT::Math::XYVector& phi0Vec() const
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
