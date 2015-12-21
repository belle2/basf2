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
#include <tracking/trackFindingCDC/numerics/numerics.h>

#include <framework/logging/Logger.h>
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
      WireLine(const Vector3D& forward, const Vector3D& backward);

      /// Returns a copy of the wire line moved by a three dimensional offset
      WireLine movedBy(const Vector3D& offset) const;

      /// Returns a copy of the wire line moved by a two dimensional offset
      WireLine movedBy(const Vector2D& offset) const;

      /// Gives the three dimensional position of the line at the given z value
      Vector3D pos3DAtZ(const double z) const
      { return Vector3D(pos2DAtZ(z), z); }

      /// Gives the two dimensional position of the line at the given z value
      Vector2D pos2DAtZ(const double z) const
      { return refPos2D() + movePerZ() * (z - refZ()); }

      /// Gives the position of the forward point
      Vector3D forward3D() const
      { return pos3DAtZ(forwardZ()) ; }

      /// Gives the xy position of the forward point
      Vector2D forward2D() const
      { return pos2DAtZ(forwardZ()) ; }

      /// Gives the position of the backward point
      Vector3D backward3D() const
      { return pos3DAtZ(backwardZ()) ; }

      /// Gives the xy position of the backward point
      Vector2D backward2D() const
      { return pos2DAtZ(backwardZ()) ; }

      /// Gives the position of the point half way between forward and backward
      Vector3D center3D() const
      { return pos3DAtZ((forwardZ() + backwardZ()) / 2) ; }

      /// Gives the xy position of the point half way between forward and backward
      Vector2D center2D() const
      { return pos2DAtZ((forwardZ() + backwardZ()) / 2) ; }

      /// Gives the tangential vector to the line
      Vector3D tangential3D() const
      { return Vector3D(movePerZ() * deltaZ(), deltaZ()); }

      /// Gives the tangential xy vector to the line
      Vector2D tangential2D() const
      { return movePerZ() * deltaZ(); }

      /// Gives the positional move in the xy projection per unit z.
      const Vector2D& movePerZ() const
      { return m_movePerZ; }

      /// Gives the forward z coodinate
      double forwardZ() const
      { return m_forwardZ; }

      /// Gives the backward z coodinate
      double backwardZ() const
      { return m_backwardZ; }

      /// Returns the difference between forward and backward z
      double deltaZ() const
      { return forwardZ() - backwardZ(); }

      /// Gives the forward azimuth angle
      double forwardPhi() const
      { return  forward2D().phi(); }

      /// Gives the backward azimuth angle
      double backwardPhi() const
      { return backward2D().phi(); }

      /// Gives the cylindrical radius of the forward position
      double forwardCylindricalR() const
      { return  forward2D().cylindricalR(); }

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

      /// Returns the closest approach on the wire to the give point
      Vector3D closest3D(const Vector3D& point) const
      { return  refPos3D() - (point - refPos3D()).parallelVector(tangential3D()); }

      /// Returns the point of closest approach to the origin on the line
      Vector3D closestToOrigin3D() const
      { return refPos3D() - refPos3D().parallelVector(tangential3D()); }

      /// Calculates the distance of the given point to the line
      double distance(const Vector3D& pos3D) const
      { return (pos3D - refPos3D()).orthogonalComp(tangential3D()); }

      /// Returns the tan lambda of the line. Also know as dz / ds.
      double tanLambda() const
      { return 1 / movePerZ().norm(); }

      /// Returns the lambda.
      double lambda() const
      { return std::atan(tanLambda()); }

      /**
       *  Returns the tangent of the opening angle between tangential vector and the z axes
       *  Also know as ds / dz.
       */
      double tanTheta() const
      { return std::atan(movePerZ().norm()); }

      /// Returns the opening angle between tangential vector and the z axes.
      double theta() const
      { return std::atan(movePerZ().norm()); }

      /// Returns the z coordinate of the point of closest approach to the z axes.
      double perigeeZ() const
      { return -refPos2D().dot(movePerZ()) / movePerZ().normSquared(); }

      /// Returns the point of closest approach to the z axes.
      Vector3D perigee3D() const
      { return pos3DAtZ(perigeeZ()); }

      /// Returns the point of closest approach to the z axes.
      Vector2D perigee2D() const
      { return refPos2D().orthogonalVector(movePerZ()); }

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

    private:
      /// Memory for the reference postion
      Vector3D m_refPos3D;

      /// Memory for the reference postion
      Vector2D m_movePerZ;

      /// Memory for the forward end z coordinate.
      double m_forwardZ = 0.0;

      /// Memory for the backward end z coordinate.
      double m_backwardZ = 0.0;

    }; // class
  } // namespace TrackFindingCDC
} // namespace Belle2
