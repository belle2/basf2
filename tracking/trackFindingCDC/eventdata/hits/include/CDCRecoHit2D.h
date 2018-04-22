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

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

#include <tracking/trackFindingCDC/topology/EStereoKind.h>
#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/numerics/ERotation.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <iosfwd>

namespace Belle2 {
  class CDCSimHit;
  class CDCHit;

  namespace TrackFindingCDC {
    class CDCTrajectory2D;
    class CDCWireHit;
    class CDCWire;
    class Vector3D;

    /**
     *  Class representing a two dimensional reconstructed hit in the central drift chamber.
     *  A recohit represents a likely point where the particle went through. It is always associated with a
     *  wire hit it seeks to reconstruct. The reconstructed point is stored as a displacement from the
     *  wire reference position assoziated with the hit. The displacement generally is as long as the drift length
     *  but must not.\n
     *  In addition the reconstructed hit takes a right left passage information which indicates if
     *  the hit wire lies to the right or to the left of the particle trajectory causing the hit. The later readily
     *  indicates a flight direction from the reconstructed hit, if a tangential approach of the trajectory to the
     *  drift circle is assumed.
     */
    class CDCRecoHit2D  {

    public:
      /// Default constructor for ROOT
      CDCRecoHit2D() = default;

      /**
       *  Constructs a reconstructed hit based on the given oriented wire hit with the given
       *  displacement from the wire reference position.
       */
      CDCRecoHit2D(const CDCRLWireHit& rlWireHit,
                   const Vector2D& recoDisp2D);

      /// Constructs a reconstructed hit based on the oriented wire hit with no displacement.
      explicit CDCRecoHit2D(const CDCRLWireHit& rlWireHit);

      /**
       *  Constructs the average of two reconstructed hit positions and snaps it to the drift circle.
       *  Averages the hit positions first. But the result will not lie on the circle, so we scale the
       *  displacement to snap onto the drift circle again. The function averages only reconstructed hits
       *  assoziated with the same right left oriented wire hit.\n
       *  If not all recostructed hits are on the same wire hit, the first hit is used.
       */
      static CDCRecoHit2D average(const CDCRecoHit2D& recoHit1,
                                  const CDCRecoHit2D& recoHit2);

      /**
       *  Constructs the average of three reconstructed hit positions and snaps it to the drift circle.
       *  Averages the hit positions first. But the result will not lie on the circle, so we scale the
       *  the displacement to snap onto the drift circle again. The function averages only reconstructed hits
       *  assoziated with the same wire hit.\n
       *  If not all recostructed hits are on the same wire, the first hit is used.
       */
      static CDCRecoHit2D average(const CDCRecoHit2D& recoHit1,
                                  const CDCRecoHit2D& recoHit2 ,
                                  const CDCRecoHit2D& recoHit3);

      /**
       *  Constructs a two dimensional reconstructed hit from an absolute position.
       *
       *  @param rlWireHit the oriented wire hit the reconstructed hit is assoziated to
       *  @param recoPos2D the absolut position of the wire
       *  @param snap      optional indicator if the displacement shall be shrank to the drift circle (default true)
       */
      static CDCRecoHit2D
      fromRecoPos2D(const CDCRLWireHit& rlWireHit, const Vector2D& recoPos2D, bool snap = true);

      /**
       *  Turns the orientation in place.
       *  Changes the sign of the right left passage information,
       *  since the position remains the same by this reversion.
       */
      void reverse();

      /// Returns the recohit with the opposite right left information.
      CDCRecoHit2D reversed() const;

      /// Getter for the alias version of the reco hit
      CDCRecoHit2D getAlias() const;

      /**
       *  Constructs a two dimensional reconstructed hit from a sim hit and the assoziated wirehit.
       *  This translates the sim hit to a reconstructed hit mainly to be able to compare the
       *  reconstructed values from the algorithm with the Monte Carlo information.
       *  It merely takes the displacement from the wire, projects it to the reference plane and
       *  scales it onto the drift circle defined by the wire.
       */
      static CDCRecoHit2D fromSimHit(const CDCWireHit* wireHit, const CDCSimHit& simHit);


      /// Make the wire hit automatically castable to its underlying cdcHit.
      operator const Belle2::CDCHit* () const
      { return static_cast<const CDCHit*>(getRLWireHit()); }


      /// Equality comparision based on the oriented wire hit and displacement.
      bool operator==(const CDCRecoHit2D& other) const
      {
        return getRLWireHit() == other.getRLWireHit() and
               getRecoDisp2D() == other.getRecoDisp2D();
      }

      /**
       *  Total ordering relation based on wire hit, left right passage information
       *  and displacement in this order of importance.
       */
      bool operator<(const CDCRecoHit2D& other) const
      {
        return getRLWireHit() <  other.getRLWireHit() or (
                 getRLWireHit() == other.getRLWireHit() and
                 getRecoDisp2D() < other.getRecoDisp2D());
      }

      /// Defines wires and the two dimensional reconstructed hits as coaligned.
      friend bool operator<(const CDCRecoHit2D& recoHit2D, const CDCWire& wire)
      { return recoHit2D.getRLWireHit() < wire; }

      /// Defines wires and the two dimensional reconstructed hits as coaligned.
      friend bool operator<(const CDCWire& wire, const CDCRecoHit2D& recoHit2D)
      { return wire < recoHit2D.getRLWireHit(); }

      /// Defines wire hits and the two dimensional reconstructed hits as coaligned.
      friend bool operator<(const CDCRecoHit2D& recoHit2D, const CDCWireHit& wireHit)
      { return recoHit2D.getRLWireHit() < wireHit; }

      /// Defines wire hits and the two dimensional reconstructed hits as coaligned.
      friend bool operator<(const CDCWireHit& wireHit, const CDCRecoHit2D& recoHit2D)
      { return wireHit < recoHit2D.getRLWireHit(); }

      /// Getter for the stereo type of the underlying wire.
      EStereoKind getStereoKind() const
      {
        return getRLWireHit().getStereoKind();
      }

      /// Indicator if the underlying wire is axial.
      bool isAxial() const
      {
        return getRLWireHit().isAxial();
      }

      /// Getter for the superlayer id.
      ISuperLayer getISuperLayer() const
      {
        return getRLWireHit().getISuperLayer();
      }

      /// Getter for the wire the reconstructed hit assoziated to.
      const CDCWire& getWire() const
      {
        return getRLWireHit().getWire();
      }

      /// Getter for the reference position of the wire.
      const Vector2D& getRefPos2D() const
      {
        return getRLWireHit().getRefPos2D();
      }

      /// Checks if the reconstructed hit is assoziated with the give wire.
      bool isOnWire(const CDCWire& wire) const
      {
        return getRLWireHit().isOnWire(wire);
      }

      /// Getter for the wire hit assoziated with the reconstructed hit.
      const CDCWireHit& getWireHit() const
      {
        return getRLWireHit().getWireHit();
      }

      /// Checks if the reconstructed hit is assoziated with the give wire hit.
      bool hasWireHit(const CDCWireHit& wireHit) const
      {
        return getRLWireHit().hasWireHit(wireHit);
      }

      /// Getter for the right left passage information.
      ERightLeft getRLInfo() const
      {
        return getRLWireHit().getRLInfo();
      }

      /// Setter the right left passage information.
      void setRLInfo(ERightLeft& rlInfo)
      {
        m_rlWireHit.setRLInfo(rlInfo);
      }

      /// Getter for the drift length at the wire reference position.
      double getRefDriftLength() const
      {
        return getRLWireHit().getRefDriftLength();
      }

      /// Setter for the drift length at the wire reference position.
      void setRefDriftLength(double driftLength, bool snapRecoPos);

      /// Getter for the drift length at the wire reference position signed with the right left passage hypotheses.
      double getSignedRefDriftLength() const
      {
        return getRLWireHit().getSignedRefDriftLength();
      }

      /// Getter for the uncertainty in the drift length at the wire reference position.
      double getRefDriftLengthVariance() const
      {
        return getRLWireHit().getRefDriftLengthVariance();
      }

      /// Getter for the position in the reference plane.
      Vector2D getRecoPos2D() const
      {
        return getRecoDisp2D() + getRefPos2D();
      }

      /// Setter for the position in the reference plane.
      void setRecoPos2D(const Vector2D& recoPos2D)
      {
        m_recoDisp2D = recoPos2D - getRefPos2D();
      }

      /// Getter for the displacement from the wire reference position.
      const Vector2D& getRecoDisp2D() const
      {
        return m_recoDisp2D;
      }

      /// Getter for the direction of flight
      Vector2D getFlightDirection2D() const
      {
        ERotation rotation = static_cast<ERotation>(-getRLInfo());
        return getRecoDisp2D().orthogonal(rotation);
      }

      /// Getter for the direction of flight relative to the position
      double getAlpha() const
      {
        return getRecoPos2D().angleWith(getFlightDirection2D());
      }

      /// Scales the displacement vector in place to lie on the dirft circle.
      void snapToDriftCircle(bool switchSide = false);

      /**
       *  Reconstruct the three dimensional position (especially of stereo hits)
       *  by determinating the z coordinate such that the reconstucted position lies on the
       *  given two dimensional trajectory as the reference reconstructed position is
       *  moved parallel to the stereo wire.
       *  For axial hits the point of closest approach on the trajectory is returned.
       */
      Vector3D reconstruct3D(const CDCTrajectory2D& trajectory2D, const double z = 0) const;

      /// Getter for the oriented wire hit assoziated with the reconstructed hit.
      const CDCRLWireHit& getRLWireHit() const
      {
        return m_rlWireHit;
      }

      /// Setter for the oriented wire hit assoziated with the reconstructed hit.
      void setRLWireHit(const CDCRLWireHit& rlWireHit)
      {
        m_rlWireHit = rlWireHit;
      }

    private:
      /// Memory for the reference to the assiziated wire hit.
      CDCRLWireHit m_rlWireHit;

      /// Memory for the displacement fo the assoziated wire reference position.
      Vector2D m_recoDisp2D;
    };


    /// Output operator. Help debugging.
    std::ostream& operator<<(std::ostream& output, const CDCRecoHit2D& recohit);
  }
}
