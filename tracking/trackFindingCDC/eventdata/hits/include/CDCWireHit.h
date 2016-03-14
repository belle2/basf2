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

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/topology/CDCWireSuperLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <cdc/dataobjects/TDCCountTranslatorBase.h>
#include <cdc/dataobjects/CDCHit.h>

#include <assert.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    const double c_simpleDriftLengthVariance  = 0.000169;

    /** Class representing a hit wire in the central drift chamber.
     *
     *  This class combines the measurement information from a CDCHit
     *  with the geometry information.
     *  It forms the basis of all other higher level tracking objects.
     *  It contains an AutomatonCell which defines the flags of usage etc.
     *  Additionally it contains references to both the CDCWire instance from the CDCWireTopology and
     *  the CDCHit from the StoreArray of the event.
     */
    class CDCWireHit {

    public:
      /// Return an instance of the used TDC count translator.
      static CDC::TDCCountTranslatorBase& getTDCCountTranslator();

      /// Default constructor for ROOT compatibility.
      CDCWireHit();

      /// Constructor for augmenting the CDCHit with the geometry information of the CDCWire.
      /** Binds to the CDCHit and the corresponding wire together and translates the TDC count to a driftlength.
       *  Also stores the index of the StoreArray
       *  from which the hit has been taken. Necessary for later translation to genfit.
       *
       *  @param  ptrHit          Reference to the CDCHit.
       *  @param  ptrTranslator   Optional xt relation used to translate the tdc count to a drift length.
       *                          If omitted use the default from the CDCWireHit.
       */
      CDCWireHit(const CDCHit* const ptrHit, CDC::TDCCountTranslatorBase* ptrTranslator = nullptr);

      /// Constructor that takes a wire ID and a driftlength at the reference. For testing only!
      CDCWireHit(const WireID& wireID,
                 const double driftLength,
                 const double driftlengthVariance = c_simpleDriftLengthVariance);

      /// Equality comparison based on the wire and the hit id.
      bool operator==(const CDCWireHit& rhs) const
      { return getWire() == rhs.getWire() and getRefDriftLength() == rhs.getRefDriftLength(); }


      /// Total ordering relation based on the wire and the hit id.
      bool operator<(const CDCWireHit& rhs) const
      {
        return
          std::make_pair(getWire(), getRefDriftLength()) <
          std::make_pair(rhs.getWire(), rhs.getRefDriftLength());
      }

      /// Defines CDCWires and CDCWireHits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCWireHit& wireHit, const CDCWire& wire)
      { return wireHit.getWire() < wire; }

      /// Defines CDCWires and CDCWireHits to be coaligned on the wire on which they are based.
      // Same as above but the other way round.
      friend bool operator<(const CDCWire& wire, const CDCWireHit& wireHit)
      { return wire < wireHit.getWire(); }

      /// Defines CDCWires and CDCWireHits to be coaligned on the wire on which they are based.
      // Same as above but with CDCWireHit as a pointer.
      friend bool operator<(const CDCWireHit* wireHit, const CDCWire& wire)
      { assert(wireHit); return  wireHit->getWire() < wire; }

      /// Defines CDCWires and CDCWireHits to be coaligned on the wire on which they are based.
      // Same as above but the other way round and with CDCWireHit as a pointer.
      friend bool operator<(const CDCWire& wire, const CDCWireHit* wireHit)
      { assert(wireHit); return wire < wireHit->getWire(); }

      /// Defines CDCWireHits and raw CDCHit to be coaligned.
      friend bool operator<(const CDCWireHit& wireHit, const CDCHit& hit)
      { return wireHit.getWire().getEWire() < hit.getID(); }

      /// Defines wire hits and raw CDCHit to be coaligned.
      friend bool operator<(const CDCHit& hit, const CDCWireHit& wireHit)
      // Same as above but the other way round.
      { return hit.getID() < wireHit.getWire().getEWire(); }

      /// Defines CDCWireSuperLayer and CDCWireHit to be coaligned on the super layer on which they are based.
      friend bool operator<(const CDCWireHit& wireHit, const CDCWireSuperLayer& wireSuperLayer)
      { return wireHit.getISuperLayer() < wireSuperLayer.getISuperLayer(); }

      /// Defines CDCWireSuperLayer and CDCWireHit to be coaligned on the super layer on which they are based.
      // Same as above but the other way round.
      friend bool operator<(const CDCWireSuperLayer& wireSuperLayer, const CDCWireHit& wireHit)
      { return wireSuperLayer.getISuperLayer() < wireHit.getISuperLayer(); }

      /** @name Mimic pointer
        */
      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference
       *  there is no way to tell if one should use the dot '.' or operator '->' for method look up.
       *  So this function defines the -> operator for the object.
       *  No matter you have a pointer or an object access is given with '->'
       *  The object is effectively equal to a pointer to itself. */
      /**@{*/
      const CDCWireHit* operator->() const
      { return this; }

      /// Allow automatic taking of the address.
      /** In places where a pointer is expected the object is implicitly casted to a
       *  pointer to itself.*/
      operator const Belle2::TrackFindingCDC::CDCWireHit* () const&
      { return this; }
      /**@}*/

      /// Getter for the CDCHit pointer into the StoreArray.
      const CDCHit* getHit() const
      { return m_hit; }

      /// Getter for the index of the hit in the StoreArray holding this hit.
      Index getStoreIHit() const
      { return getHit() ? getHit()->getArrayIndex() : INVALID_INDEX; }

      /// Getter for the CDCWire the hit is located on.
      const CDCWire& getWire() const
      { return *m_wire; }

      /// Getter for the WireID of the wire the hit is located on.
      const WireID& getWireID() const
      { return getWire().getWireID(); }

      /// Getter for the stereo type of the underlying wire.
      StereoType getStereoType() const
      { return getWire().getStereoType(); }

      /// Indicator if the underlying wire is axial.
      bool isAxial() const
      { return getWire().isAxial(); }

      /// Getter for the super layer id.
      ILayerType getISuperLayer() const
      { return getWire().getISuperLayer(); }

      /// The two dimensional reference position (z=0) of the underlying wire.
      const Vector2D& getRefPos2D() const
      { return getWire().getRefPos2D(); }

      /// The three dimensional reference position of the underlying wire.
      const Vector3D& getRefPos3D() const
      { return getWire().getRefPos3D(); }

      /// The distance from the beam line at reference position of the underlying wire.
      double getRefCylindricalR() const
      { return getWire().getRefCylindricalR(); }

      /// Getter for the drift length at the reference position of the wire.
      double getRefDriftLength() const
      { return m_refDriftLength; }

      /// Getter for the variance of the drift length at the reference position of the wire.
      double getRefDriftLengthVariance() const
      { return m_refDriftLengthVariance; }

      /// Checks if the wire hit is based on the given wire.
      bool isOnWire(const CDCWire& wire) const
      { return getWire() == wire; }

      /** Reconstructs a position of primary ionisation on the drift circle.
       *
       *  The result is the position of closest approach on the drift circle to the trajectory.
       *
       *  All positions and the trajectory are interpreted to lie at z=0.
       *  Also the right left passage hypotheses does not play a role in
       *  the reconstruction in any way.
       */
      Vector2D reconstruct2D(const CDCTrajectory2D& trajectory2D) const;

      /** Attempts to reconstruct a three dimensional position (especially of stereo hits).
       *
       *  This method makes a distinct difference between axial and stereo hits:
       *  * Stereo hits are moved out of the reference plane such that the
       *    oriented drift circle meets the trajectory in one point. Therefore the
       *    left right passage hypothesis has to be taken into account
       *  * For axial hits the reconstructed position is undefined in the z coordinate.
       *    Also the drift circle cannot be moved such that it would meet the
       *    trajectory. Hence we default to the result of reconstruct2D, which
       *    yield the closest approach of the drift circle to the trajectory
       *    in the reference plane.
       */
      Vector3D reconstruct3D(const CDCTrajectory2D& trajectory2D, const ERightLeft rlInfo) const;


      /// String output operator for wire hit objects to help debugging
      friend std::ostream& operator<<(std::ostream& output, const CDCWireHit& wirehit)
      { return output << "CDCWireHit(" << wirehit.getWire() << ", drift length=" << wirehit.getRefDriftLength() << ")"; }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell()
      { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const
      { return m_automatonCell; }

    private:
      /// Memory for the automaton cell.
      AutomatonCell m_automatonCell;

      /// Memory for the drift length at the wire reference point.
      double m_refDriftLength = 0;

      /// Memory for the variance of the drift length at the wire reference point.
      double m_refDriftLengthVariance = c_simpleDriftLengthVariance;

      /// Memory for the CDCWire pointer.
      const CDCWire* m_wire = nullptr;

      /// Memory for the CDCHit pointer.
      const CDCHit* m_hit = nullptr;

    }; //class CDCWireHit

  } // namespace TrackFindingCDC
} // namespace Belle2
