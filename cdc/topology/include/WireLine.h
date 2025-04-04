/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/geometry/VectorUtil.h>
// #include <tracking/trackingUtilities/geometry/Vector3D.h>
#include <Math/Vector2D.h>
#include <Math/Vector3D.h>

#include <cmath>

namespace Belle2 {
  namespace TrackingUtilities {
    /**
     *  A three dimensional limited line represented by its closest approach to the z-axes (reference position ) and its skew parameter.
     *  The representation is only suitable for lines out of the xy plane.
     *  The endpoints are stored encoded by their z position.
     *  The end point with the higher z value is called forward, the one with the smaller is called backward.
     *  This class is mainly used to describe the cdc wires with as few parameters as possible and
     *  simplifies the retrieval of the two dimensional track reference position, which is taken at
     *  the closest approach to the beam z-axes.
     *  @brief A three dimensional limited line
     */
    class WireLine  {

    public:
      /// Default constructor initialising to all members to zero
      WireLine()
      {}

      /// Constructor for a wire line between forward and backward point
      WireLine(const ROOT::Math::XYZVector& forward, const ROOT::Math::XYZVector& backward, double sagCoeff);

      /// Returns a copy of the wire line moved by a three dimensional offset
      WireLine movedBy(const ROOT::Math::XYZVector& offset) const;

      /// Returns a copy of the wire line moved by a two dimensional offset
      WireLine movedBy(const ROOT::Math::XYVector& offset) const;

      /// Gives the three dimensional position *without* wire sag effect of the line at the given z value
      ROOT::Math::XYZVector nominalPos3DAtZ(const double z) const
      {
        const ROOT::Math::XYVector& tmp = nominalPos2DAtZ(z);
        return ROOT::Math::XYZVector(tmp.X(), tmp.Y(), z);
      }

      /// Gives the two dimensional position *without* wire sag effect of the line at the given z value
      ROOT::Math::XYVector nominalPos2DAtZ(const double z) const
      { return refPos2D() + nominalMovePerZ() * z; }

      /// Gives the three dimensional position *with* wire sag effect of the line at the given z value
      ROOT::Math::XYZVector sagPos3DAtZ(const double z) const
      {
        const ROOT::Math::XYVector& tmp = sagPos2DAtZ(z);
        return ROOT::Math::XYZVector(tmp.X(), tmp.Y(), z);
      }

      /// Gives the two dimensional position *with* wire sag effect of the line at the given z value
      ROOT::Math::XYVector sagPos2DAtZ(const double z) const
      {
        /// Efficient version without divisions
        ROOT::Math::XYVector pos2D = nominalPos2DAtZ(z);
        double xzFactorSquared = 1 + nominalMovePerZ().X() * nominalMovePerZ().X();
        double sagY = - sagCoeff() * xzFactorSquared * (forwardZ() - z) * (z - backwardZ());
        pos2D.SetY(pos2D.Y() + sagY);
        return pos2D;
      }

      /// Gives the positional move in the xy projection per unit z.
      const ROOT::Math::XYVector& nominalMovePerZ() const
      { return m_nominalMovePerZ; }

      /// Gives the two dimensional position *with* wire sag effect of the line at the given z value
      ROOT::Math::XYVector sagMovePerZ(const double z) const
      {
        /// Efficient version without divisions
        ROOT::Math::XYVector movePerZ = nominalMovePerZ();
        double xzFactorSquared = 1 + nominalMovePerZ().X() * nominalMovePerZ().X();
        double sagDYDZ = - sagCoeff() * xzFactorSquared * (forwardZ() + backwardZ() - 2 * z);
        movePerZ.SetY(movePerZ.Y() + sagDYDZ);
        return movePerZ;
      }

      /// Calculates the distance of the given point to the wire *without* wire sag effect
      double nominalDistance(const ROOT::Math::XYZVector& pos3D) const
      {
        return VectorUtil::orthogonalComp((pos3D - refPos3D()), ROOT::Math::XYZVector(nominalMovePerZ().X(), nominalMovePerZ().Y(), 1));
      }

      /// Calculates the distance of the given point to the wire *with* wire sag effect
      double sagDistance(const ROOT::Math::XYZVector& pos3D) const
      {
        const ROOT::Math::XYZVector& wirePos3D = sagPos3DAtZ(pos3D.z());
        const ROOT::Math::XYVector& tmp = sagMovePerZ(pos3D.z());
        const ROOT::Math::XYZVector movePerZ(tmp.X(), tmp.Y(), 1);
        return VectorUtil::orthogonalComp((pos3D - wirePos3D), movePerZ);
      }

      /// Returns the closest approach on the wire *without* wire sag effect to the give point
      ROOT::Math::XYZVector nominalClosest3D(const ROOT::Math::XYZVector& point) const
      {
        return refPos3D() - VectorUtil::parallelVector((point - refPos3D()), ROOT::Math::XYZVector(nominalMovePerZ().X(),
                                                       nominalMovePerZ().Y(), 1));
      }

      /// Returns the closest approach on the wire *with* wire sag effect to the give point
      ROOT::Math::XYZVector sagClosest3D(const ROOT::Math::XYZVector& point) const
      {
        const ROOT::Math::XYZVector& wirePos3D = sagPos3DAtZ(point.z());
        const ROOT::Math::XYVector& tmp = sagMovePerZ(point.z());
        const ROOT::Math::XYZVector movePerZ(tmp.X(), tmp.Y(), 1);
        return  wirePos3D - VectorUtil::parallelVector((point - wirePos3D), movePerZ);
      }

      /// Gives the position of the forward point
      ROOT::Math::XYZVector forward3D() const
      { return nominalPos3DAtZ(forwardZ()); }

      /// Gives the xy position of the forward point
      ROOT::Math::XYVector forward2D() const
      { return nominalPos2DAtZ(forwardZ()); }

      /// Gives the position of the backward point
      ROOT::Math::XYZVector backward3D() const
      { return nominalPos3DAtZ(backwardZ()); }

      /// Gives the xy position of the backward point
      ROOT::Math::XYVector backward2D() const
      { return nominalPos2DAtZ(backwardZ()); }

      /// Getter for the vector from backward to  the forward position
      ROOT::Math::XYZVector wireVector() const
      {
        const ROOT::Math::XYVector& tmp = nominalMovePerZ() * deltaZ();
        return ROOT::Math::XYZVector(tmp.X(), tmp.Y(), deltaZ());
      }

      /// Gives the forward z coordinate
      double forwardZ() const
      { return m_forwardZ; }

      /// Gives the backward z coordinate
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
      { return forward2D().Phi(); }

      /// Gives the backward azimuth angle
      double backwardPhi() const
      { return backward2D().Phi(); }

      /// Gives the cylindrical radius of the forward position
      double forwardCylindricalR() const
      { return forward2D().R(); }

      /// Gives the cylindrical radius of the backward position
      double backwardCylindricalR() const
      { return backward2D().R(); }

      /// Gives the azimuth angle of the forward position relative to the reference position
      double forwardPhiToRef() const
      { return VectorUtil::Angle(forward2D(), refPos2D()); }

      /// Gives the azimuth angle of the backward position relative to the reference position
      double backwardPhiToRef() const
      { return VectorUtil::Angle(backward2D(), refPos2D()); }

      /// Gives the azimuth angle difference from backward to forward position
      /* backwardToForwardAngle means how far the backward position has to be rotated in the xy projection
         in the mathematical positive sense that it seems to be coaligned with the forward position. */
      double backwardToForwardAngle() const
      { return VectorUtil::Angle(backward2D(), forward2D()); }

      /// Returns the nominal tan lambda of the line. Also know as dz / ds.
      double tanLambda() const
      { return 1 / nominalMovePerZ().R(); }

      /// Returns the nominal lambda angle of the line
      double lambda() const
      { return std::atan(tanLambda()); }

      /**
       *  Returns the tangent of the opening angle between tangential vector and the z axes
       *  Also know as ds / dz.
       */
      double tanTheta() const
      { return std::atan(nominalMovePerZ().R()); }

      /// Returns the nominal opening angle between tangential vector and the z axes.
      double theta() const
      { return std::atan(nominalMovePerZ().R()); }

      /// Returns the z coordinate of the point of nominal closest approach to the z axes.
      double nominalPerigeeZ() const
      { return -refPos2D().Dot(nominalMovePerZ()) / nominalMovePerZ().Mag2(); }

      /// Returns the point of nominal closest approach to the z axes.
      ROOT::Math::XYZVector nominalPerigee3D() const
      { return nominalPos3DAtZ(nominalPerigeeZ()); }

      /// Returns the point of nominal closest approach to the z axes.
      ROOT::Math::XYVector nominalPerigee2D() const
      { return VectorUtil::orthogonalVector(refPos2D(), nominalMovePerZ()); }

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
      { return m_refPos3D.Perp2(); }

      /// Returns the xy vector of the reference position
      const ROOT::Math::XYVector& refPos2D() const
      { return m_refPos2D; }

      /// Returns the reference position
      const ROOT::Math::XYZVector& refPos3D() const
      { return m_refPos3D; }

      /// Returns the wire sag coefficient due to gravity
      double sagCoeff() const
      { return m_sagCoeff; }

    private:
      /// Memory for the reference position
      ROOT::Math::XYZVector m_refPos3D;

      /// Memory for the 2D reference position (to avoid [-Wreturn-stack-address] in refPos2D())
      ROOT::Math::XYVector m_refPos2D;

      /// Memory for the nominal movement of the xy position per z unit off the reference.
      ROOT::Math::XYVector m_nominalMovePerZ;

      /// Memory for the forward end z coordinate.
      double m_forwardZ = 0.0;

      /// Memory for the backward end z coordinate.
      double m_backwardZ = 0.0;

      /// Memory for the wire sag coefficient.
      double m_sagCoeff = 0.0;

    };
  }
}
