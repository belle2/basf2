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

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     *  A three dimensional limited line represented by its closest approach to the z-axes (reference position ) and its skew parameter.
     *  The representation is only suitable for lines out of the xy plane.
     *  The endpoints are stored encoded by their z position.
     *  The end point with the higher z value is called forward, the one with the smaller is called backward.
     *  This class is mainly used to descripe the cdc wires with as few parameters as possible and
     *  simplifies the retrival of the two dimensional track reference position, which is taken at
     *  the closest approach to the beam z-axes.
     *  @brief A three dimensional limited line
     */
    class WireLine  {

    public:
      /// Default constructor initialising to all members to zero
      WireLine()
      {}

      /// Constuctor for a wire line between forward and backward point
      WireLine(const Vector3D& forward, const Vector3D& backward, double sagCoeff);

      /// Returns a copy of the wire line moved by a three dimensional offset
      WireLine movedBy(const Vector3D& offset) const;

      /// Returns a copy of the wire line moved by a two dimensional offset
      WireLine movedBy(const Vector2D& offset) const;

      /// Gives the three dimensional position *without* wire sag effect of the line at the given z value
      Vector3D nominalPos3DAtZ(const double z) const
      { return Vector3D(nominalPos2DAtZ(z), z); }

      /// Gives the two dimensional position *without* wire sag effect of the line at the given z value
      Vector2D nominalPos2DAtZ(const double z) const
      { return refPos2D() + nominalMovePerZ() * z; }

      /// Gives the three dimensional position *with* wire sag effect of the line at the given z value
      Vector3D sagPos3DAtZ(const double z) const
      { return Vector3D(sagPos2DAtZ(z), z); }

      /// Gives the two dimensional position *with* wire sag effect of the line at the given z value
      Vector2D sagPos2DAtZ(const double z) const
      {
        /// Efficient version without divisions
        Vector2D pos2D = nominalPos2DAtZ(z);
        double xzFactorSquared = 1 + nominalMovePerZ().x() * nominalMovePerZ().x();
        double sagY = - sagCoeff() * xzFactorSquared * (forwardZ() - z) * (z - backwardZ());
        pos2D.setY(pos2D.y() + sagY);
        return pos2D;
      }

      /// Gives the positional move in the xy projection per unit z.
      const Vector2D& nominalMovePerZ() const
      { return m_nominalMovePerZ; }

      /// Gives the two dimensional position *with* wire sag effect of the line at the given z value
      Vector2D sagMovePerZ(const double z) const
      {
        /// Efficient version without divisions
        Vector2D movePerZ = nominalMovePerZ();
        double xzFactorSquared = 1 + nominalMovePerZ().x() * nominalMovePerZ().x();
        double sagDYDZ = - sagCoeff() * xzFactorSquared * (forwardZ() + backwardZ() - 2 * z);
        movePerZ.setY(movePerZ.y() + sagDYDZ);
        return movePerZ;
      }

      /// Calculates the distance of the given point to the wire *without* wire sag effect
      double nominalDistance(const Vector3D& pos3D) const
      { return (pos3D - refPos3D()).orthogonalComp(Vector3D(nominalMovePerZ(), 1)); }

      /// Calculates the distance of the given point to the wire *with* wire sag effect
      double sagDistance(const Vector3D& pos3D) const
      {
        Vector3D wirePos3D = sagPos3DAtZ(pos3D.z());
        Vector3D movePerZ(sagMovePerZ(pos3D.z()), 1);
        return (pos3D - wirePos3D).orthogonalComp(movePerZ);
      }

      /// Returns the closest approach on the wire *without* wire sag effect to the give point
      Vector3D nominalClosest3D(const Vector3D& point) const
      { return  refPos3D() - (point - refPos3D()).parallelVector(Vector3D(nominalMovePerZ(), 1)); }

      /// Returns the closest approach on the wire *with* wire sag effect to the give point
      Vector3D sagClosest3D(const Vector3D& point) const
      {
        Vector3D wirePos3D = sagPos3DAtZ(point.z());
        Vector3D movePerZ(sagMovePerZ(point.z()), 1);
        return  wirePos3D - (point - wirePos3D).parallelVector(movePerZ);
      }

      /// Gives the position of the forward point
      Vector3D forward3D() const
      { return nominalPos3DAtZ(forwardZ()); }

      /// Gives the xy position of the forward point
      Vector2D forward2D() const
      { return nominalPos2DAtZ(forwardZ()); }

      /// Gives the position of the backward point
      Vector3D backward3D() const
      { return nominalPos3DAtZ(backwardZ()); }

      /// Gives the xy position of the backward point
      Vector2D backward2D() const
      { return nominalPos2DAtZ(backwardZ()); }

      /// Getter for the vector from backward to  the forward position
      Vector3D wireVector() const
      { return Vector3D(nominalMovePerZ() * deltaZ(), deltaZ()); }

      /// Gives the forward z coodinate
      double forwardZ() const
      { return m_forwardZ; }

      /// Gives the backward z coodinate
      double backwardZ() const
      { return m_backwardZ; }

      /// Returns the difference between forward and backward z
      double deltaZ() const
      { return forwardZ() - backwardZ(); }

      /// Returns the amount how much the given z position is outside the bounds in units of the wire length
      double outOfZBoundsFactor(double z) const
      { return std::fmax(backwardZ() - z, z - forwardZ()) / deltaZ(); }

      /// Gives the forward azimuth angle
      double forwardPhi() const
      { return forward2D().phi(); }

      /// Gives the backward azimuth angle
      double backwardPhi() const
      { return backward2D().phi(); }

      /// Gives the cylindrical radius of the forward position
      double forwardCylindricalR() const
      { return forward2D().cylindricalR(); }

      /// Gives the cylindrical radius of the backward position
      double backwardCylindricalR() const
      { return backward2D().cylindricalR(); }

      /// Gives the azimuth angle of the forward position relative to the reference position
      double forwardPhiToRef() const
      { return forward2D().angleWith(refPos2D()); }

      /// Gives the azimuth angle of the backward position relative to the reference position
      double backwardPhiToRef() const
      { return backward2D().angleWith(refPos2D()); }

      /// Gives the azimuth angle difference from backward to forward position
      /* backwardToForwardAngle means how far the backward position has to be rotated in the xy projection
         in the mathematical positiv sense that it seems to be coaligned with the forward position. */
      double backwardToForwardAngle() const
      { return backward2D().angleWith(forward2D()) ; }

      /// Returns the nominal tan lambda of the line. Also know as dz / ds.
      double tanLambda() const
      { return 1 / nominalMovePerZ().norm(); }

      /// Returns the nominal lambda angle of the line
      double lambda() const
      { return std::atan(tanLambda()); }

      /**
       *  Returns the tangent of the opening angle between tangential vector and the z axes
       *  Also know as ds / dz.
       */
      double tanTheta() const
      { return std::atan(nominalMovePerZ().norm()); }

      /// Returns the nominal opening angle between tangential vector and the z axes.
      double theta() const
      { return std::atan(nominalMovePerZ().norm()); }

      /// Returns the z coordinate of the point of nominal closest approach to the z axes.
      double nominalPerigeeZ() const
      { return -refPos2D().dot(nominalMovePerZ()) / nominalMovePerZ().normSquared(); }

      /// Returns the point of nominal closest approach to the z axes.
      Vector3D nominalPerigee3D() const
      { return nominalPos3DAtZ(nominalPerigeeZ()); }

      /// Returns the point of nominal closest approach to the z axes.
      Vector2D nominalPerigee2D() const
      { return refPos2D().orthogonalVector(nominalMovePerZ()); }

      /// Returns the the x coordinate of the reference point.
      double refX() const
      { return m_refPos3D.x(); }

      /// Returns the the y coordinate of the reference point.
      double refY() const
      { return m_refPos3D.y(); }

      /// Returns the the z coordinate of the reference point.
      double refZ() const
      { return m_refPos3D.z(); }

      /// Returns the cylindrical radius of the reference position
      double refCylindricalRSquared() const
      { return m_refPos3D.cylindricalRSquared(); }

      /// Returns the xy vector of the reference position
      const Vector2D& refPos2D() const
      { return m_refPos3D.xy(); }

      /// Returns the reference position
      const Vector3D& refPos3D() const
      { return m_refPos3D; }

      /// Returns the wire sag coefficient due to gravity
      double sagCoeff() const
      { return m_sagCoeff; }

    private:
      /// Memory for the reference postion
      Vector3D m_refPos3D;

      /// Memory for the nominal movement of the xy position per z unit off the reference.
      Vector2D m_nominalMovePerZ;

      /// Memory for the forward end z coordinate.
      double m_forwardZ = 0.0;

      /// Memory for the backward end z coordinate.
      double m_backwardZ = 0.0;

      /// Memory for the wire sag coeffiecent.
      double m_sagCoeff = 0.0;

    };
  }
}
