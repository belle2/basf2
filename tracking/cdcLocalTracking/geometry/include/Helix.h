/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef HELIX_H
#define HELIX_H

#include <cmath>

#include "TVectorD.h"

#include <tracking/cdcLocalTracking/rootification/SwitchableRootificationBase.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include "HelixParameterIndex.h"

#include "Vector2D.h"
#include "Line2D.h"
#include "BoundSkewLine.h"
#include "PerigeeCircle.h"

namespace Belle2 {

  namespace CDCLocalTracking {

    /// Extension of the generalized circle also caching the perigee coordinates.
    class Helix : public CDCLocalTracking::SwitchableRootificationBase {

    public:

      /// Default constructor for ROOT compatibility.
      Helix() :
        m_circleXY(),
        m_lineSZ()
      {;}

      /// Constructor taking all stored parameters for internal use.
      Helix(const PerigeeCircle& circleXY,
            const Line2D& lineSZ) :
        m_circleXY(circleXY),
        m_lineSZ(lineSZ)
      {;}

      explicit Helix(const TVectorD& parameters) :
        m_circleXY(PerigeeCircle::fromPerigeeParameters(parameters(iCurv), parameters(iPhi0), parameters(iI))),
        m_lineSZ(Line2D::fromSlopeIntercept(parameters(iSZ), parameters(iZ0)))
      {;}

      Helix(const FloatType& curvature,
            const FloatType& tangentialPhi,
            const FloatType& impact,
            const FloatType& szSlope,
            const FloatType& z0) :
        m_circleXY(PerigeeCircle::fromPerigeeParameters(curvature, tangentialPhi, impact)),
        m_lineSZ(Line2D::fromSlopeIntercept(szSlope, z0))
      {;}


      Helix(const FloatType& curvature,
            const Vector2D& tangential,
            const FloatType& impact,
            const FloatType& szSlope,
            const FloatType& z0) :
        m_circleXY(PerigeeCircle::fromPerigeeParameters(curvature, tangential, impact)),
        m_lineSZ(Line2D::fromSlopeIntercept(szSlope, z0))
      {;}


      /// Empty deconstructor
      ~Helix() {;}

      /// Sets all circle parameters to zero.
      void setNull() {
        m_circleXY.setNull();
        m_lineSZ.setNull();
      }

      bool isNull() const
      { return circleXY().isNull() and lineSZ().isNull(); }

      /// Flips the travel direction of the helix in place, pivot point is unchanged.
      inline void reverse() {
        m_circleXY.reverse();
        //Invert the travel distance scale, but not the z scale.
        m_lineSZ.flipFirst();
      }

      /// Returns a copy of the helix with flips the travel direction, pivot point is the same.
      inline Helix reversed() const
      { return Helix(circleXY().reversed(), lineSZ().flippedFirst()); }

    public:
      /// Calculates the perpendicular travel distance at which the helix has the closest approach to the given point.
      FloatType closestAtPerpS(const Vector3D& point) const;

      /// Calculates the point of closest approach on the helix to the given point.
      Vector3D closest(const Vector3D& point) const {
        FloatType perpS = closestAtPerpS(point);
        return atPerpS(perpS);
      }

      //FloatType distance(const Vector3D& point) const;
      //FloatType lengthOnCurve(const Vector3D& from, const Vector3D& to) const;
      //Vector3D samePolarR(const Vector3D& point) const;
      //Vector3D samePolarRForwardOf(const Vector3D& startPoint, const FloatType& polarR) const;

      /// Moves the coordinates system by the given vector. Updates support point in place.
      void passiveMoveBy(const Vector3D& by) {
        // First keep the necessary shift of the perpendicular travel distance to the new support point.
        FloatType byS = circleXY().lengthOnCurve(perigeeXY(), by.xy());
        m_circleXY.passiveMoveBy(by.xy());
        Vector2D bySZ(byS, by.z());
        m_lineSZ.passiveMoveBy(bySZ);
      }


      /// Computes the Jacobi matrix for a move of the coordinate system by the given vector.
      TMatrixD passiveMoveByJacobian(const Vector3D& by) const;

      /// Shifts the szSlope and z0 by the given amount. Method is specific to the corrections in the fusion fit.
      void shiftSZSlopeIntercept(const FloatType& szSlopeShift, const FloatType& zShift) {
        FloatType z0 = m_lineSZ.intercept();
        FloatType szSlope = m_lineSZ.slope();
        m_lineSZ.setSlopeIntercept(szSlope + szSlopeShift, z0 + zShift);
      }

      /// Calculates the point, which lies at the give perpendicular travel distance (counted from the perigee)
      Vector3D atPerpS(const FloatType& perpS) const
      { return Vector3D(circleXY().atPerpS(perpS), lineSZ().map(perpS)); }

      /// Calculates the point, which lies at the given z coordinate
      Vector3D atZ(const FloatType& z) const
      { return Vector3D(xyAtZ(z), z); }

      /// Calculates the point, which lies at the given z coordinate
      Vector2D xyAtZ(const FloatType& z) const
      { return Vector2D(circleXY().atPerpS(lineSZ().inverseMap(z))); }


      /// Gives the minimal polar r the circle reaches (unsigned)
      inline FloatType minimalPolarR() const
      { return circleXY().minimalPolarR(); }

      /// Gives the maximal polar r the circle reaches
      inline FloatType maximalPolarR() const
      { return circleXY().maximalPolarR(); }

      /// Getter for the signed curvature in the xy projection.
      inline const FloatType& curvatureXY() const
      { return circleXY().curvature(); }

      /// Getter for the signed distance to the z axes at the support point.
      FloatType impactXY() const
      { return circleXY().impact(); }

      /// Getter for the absolute distance to the z axes at the support point
      FloatType d0() const
      { return impactXY(); }

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
      FloatType szSlope() const
      { return m_lineSZ.slope(); }

      /// Getter for the proportinality factor from arc length in xy space to z.
      FloatType cotTheta() const
      { return szSlope(); }

      /// Getter for the proportinality factor from arc length in xy space to z.
      FloatType tanLambda() const
      { return szSlope(); }

      /// Getter for z coordinate at the support point of the helix.
      FloatType z0() const
      { return m_lineSZ.intercept(); }

      /// Getter for the distance in z at which the two points on the helix coincide in the xy projection.
      FloatType zPeriod() const
      { return lineSZ().map(perimeterXY()); }

      /// Getter for the perimeter of the circle in the xy projection
      FloatType perimeterXY() const
      { return circleXY().perimeter(); }

      /// Getter for the unit three dimensional tangential vector at the support point of the helix.
      Vector3D tangential() const {
        Vector3D result(tangentialXY(), szSlope());
        result.normalize();
        return result;
      }

      /// Getter for the tangential vector in the xy projection at the support point of the helix.
      const Vector2D& tangentialXY() const
      { return circleXY().tangential(); }

      /// Getter for the polar angle of the tangential vector at the support point of the helix.
      const FloatType& tangentialPhi() const
      { return circleXY().tangentialPhi(); }

      /// Getter for the polar angle of the tangential vector at the support point of the helix.
      const  FloatType& phi0() const
      { return tangentialPhi(); }

      /// Getter for the five helix parameters in the order defined by HelixParameterIndex.h and PerigeeParameterIndex.h
      TVectorD parameters() const {
        TVectorD result(iCurv, iZ0);
        result(iCurv) = curvatureXY();
        result(iPhi0) = phi0();
        result(iI) = impactXY();
        result(iSZ) = szSlope();
        result(iZ0) = z0();
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

      /// ROOT Macro to make Helix a ROOT class.
      CDCLOCALTRACKING_SwitchableClassDef(Helix, 1);

    }; //class


  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // HELIX_H
