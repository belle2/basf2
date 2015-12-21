/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/numerics/ERightLeft.h>

namespace Belle2 {
  class CDCSimHit;

  namespace TrackFindingCDC {

    /** Class representing an oriented hit wire including a hypotheses
     *  whether the causing track passes left or right.
     *  The right left information is freely setable.
     *  In constrast to the CDCRLWireHits the instances are not managed
     *  by the CDCWireHitTopology, which means that they can be copied and created.
     *  More than one CDCRLTaggedWireHit can point to one CDCWireHit.
     *  For more information see @sa CDCWireHit.
     */
    class CDCRLTaggedWireHit {
    public:
      /** Constructs an oriented wire hit.
       *  @param wireHit the wire hit the oriented hit is associated with.
       *  @param rlInfo the right left passage information of the _wire_ relative to the track */
      CDCRLTaggedWireHit(const CDCWireHit* wireHit,
                         ERightLeft rlInfo = ERightLeft::c_Unknown);

      /// Constructs an oriented wire hit from a CDCSimHit and the associated wirehit.
      /** This translates the sim hit to an oriented wire hit mainly to be able to compare the
       *  reconstructed values from the algorithm with the Monte Carlo information.
       *  It merely evaluates, if the true trajectory passes right or left of the wire. */
      static CDCRLTaggedWireHit fromSimHit(const CDCWireHit* wirehit, const CDCSimHit& simhit);

      /// Returns the oriented wire hit with the opposite right left information.
      CDCRLTaggedWireHit reversed() const
      { return CDCRLTaggedWireHit(m_wireHit, NRightLeft::reversed(m_rlInfo)); }

      /// Swiches the right left passage to its opposite inplace.
      void reverse()
      { m_rlInfo = NRightLeft::reversed(m_rlInfo); }

      /// Equality comparison based on wire hit, left right passage information.
      bool operator==(const CDCRLTaggedWireHit& rhs) const
      { return getWireHit() == rhs.getWireHit() and getRLInfo() == rhs.getRLInfo(); }

      /** Total ordering relation based on
       *  wire hit and left right passage information in this order of importance.
       */
      bool operator<(const CDCRLTaggedWireHit& rhs) const
      {
        return getWireHit() <  rhs.getWireHit() or (
                 getWireHit() == rhs.getWireHit() and (
                   getRLInfo() < rhs.getRLInfo()));
      }

      /// Defines wires and oriented wire hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCRLTaggedWireHit& rlWireHit, const CDCWire& wire)
      { return rlWireHit.getWire() < wire; }

      /// Defines wires and oriented wire hits to be coaligned on the wire on which they are based.
      // Same as above but the other way round.
      friend bool operator<(const CDCWire& wire, const CDCRLTaggedWireHit& rlWireHit)
      { return wire < rlWireHit.getWire(); }

      /** Defines wire hits and oriented wire hits to be coaligned on the wire hit
       *  on which they are based.
       */
      friend bool operator<(const CDCRLTaggedWireHit& rlWireHit, const CDCWireHit& wireHit)
      { return rlWireHit.getWireHit() < wireHit; }

      /** Defines wire hits and oriented wire hits to be coaligned on the wire hit
       * on which they are based.
       */
      // Same as above but the other way round.
      friend bool operator<(const CDCWireHit& wireHit, const CDCRLTaggedWireHit& rlWireHit)
      { return wireHit < rlWireHit.getWireHit(); }

      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference
       *  there is no way to tell if one should use the dot '.' or operator '->' for method look up.
       *  So this function defines the -> operator for the object.
       *  No matter you have a pointer or an object access is given with '->'
       *  The object is effectively equal to a pointer to itself.
       */
      const CDCRLTaggedWireHit* operator->() const
      { return this; }

      /// Make the wire hit automatically castable to its underlying cdcHit.
      operator const Belle2::CDCHit* () const
      { return getWireHit().getHit(); }

      /// Getter for the CDCHit pointer into the StoreArray.
      const CDCHit* getHit() const
      { return getWireHit().getHit(); }

      /// Getter for the wire the oriented hit associated to.
      const CDCWire& getWire() const
      { return getWireHit().getWire(); }

      /// Checks if the oriented hit is associated with the give wire.
      bool isOnWire(const CDCWire& wire) const
      { return getWire() == wire; }

      /// Getter for the WireID of the wire the hit is located on.
      const WireID& getWireID() const
      { return getWire().getWireID(); }

      /// Getter for the superlayer id.
      ISuperLayer getISuperLayer() const
      { return getWire().getISuperLayer(); }

      /// Getter for the stereo type of the underlying wire.
      EStereoKind getStereoKind() const
      { return getWire().getStereoKind(); }

      /// Indicator if the underlying wire is axial.
      bool isAxial() const
      { return getWire().isAxial(); }

      /// The two dimensional reference position of the underlying wire.
      const Vector2D& getRefPos2D() const
      { return getWire().getRefPos2D(); }

      /// The distance from the beam line at reference position of the underlying wire.
      double getRefCylindricalR() const
      { return getWire().getRefCylindricalR(); }

      /// Getter for the wire hit associated with the oriented hit.
      const CDCWireHit& getWireHit() const
      { return *m_wireHit; }

      /// Checks if the oriented hit is associated with the give wire hit.
      bool hasWireHit(const CDCWireHit& wirehit) const
      { return getWireHit() == wirehit; }

      /// Getter for the  drift length at the reference position of the wire.
      double getRefDriftLength() const
      { return getWireHit().getRefDriftLength(); }

      /// Getter for the  drift length at the reference position of the wire.
      double getSignedRefDriftLength() const
      { return ((ESign)(getRLInfo())) * getRefDriftLength(); }

      /// Getter for the variance of the drift length at the reference position of the wire.
      double getRefDriftLengthVariance() const
      { return getWireHit().getRefDriftLengthVariance(); }

      /// Getter for the right left passage information.
      ERightLeft getRLInfo() const
      { return m_rlInfo; }

      /// Setter for the right left passage information.
      void setRLInfo(const ERightLeft rlInfo)
      { m_rlInfo = rlInfo; }

      /** Reconstructs a position of primary ionisation on the drift circle.
       *
       *  The result is the position of closest approach on the drift circle to the trajectory.
       *
       *  All positions and the trajectory are interpreted to lie at z=0.
       *  Also the right left passage hypotheses does not play a role in
       *  the reconstruction in any way.
       */
      Vector2D reconstruct2D(const CDCTrajectory2D& trajectory2D) const
      { return getWireHit().reconstruct2D(trajectory2D); }

      /** Attempts to reconstruct a three dimensional position (especially of stereo hits).
       *
       *  This method makes a distinct difference between axial and stereo hits:
       *  * Stereo hits are moved out of the reference plane such that the
       *    oriented drift circle meets the trajectory in one point. Therefore the
       *    left right passage hypothesis has to be taken into account
       *  * For axial hits the reconstructed position is ambiguous in the z coordinate.
       *    Also the drift circle cannot moved such that it would meet the
       *    trajectory. Hence we default to the result of reconstruct2D, which
       *    yield the closest approach of the drift circle to the trajectory
       *    in the reference plane.
       */
      Vector3D reconstruct3D(const CDCTrajectory2D& trajectory2D) const
      { return getWireHit().reconstruct3D(trajectory2D, getRLInfo()); }

      /// Output operator. Help debugging.
      friend std::ostream& operator<<(std::ostream& output, const CDCRLTaggedWireHit& rlWireHit)
      {
        output << "CDCRLTaggedWireHit(" << rlWireHit.getWireHit() << ","
               << static_cast<typename std::underlying_type<ERightLeft>::type>(rlWireHit.getRLInfo()) << ")" ;
        return output;
      }

    private:
      /// Memory for the reference to the assiziated wire hit.
      const CDCWireHit* m_wireHit;

      /// Memory for the right left passage information of the oriented wire hit.
      NRightLeft::ERightLeft m_rlInfo;

    }; //class CDCRLTaggedWireHit
  } // end namespace TrackFindingCDC
} // end namespace Belle2
