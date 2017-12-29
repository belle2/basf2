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

#include <tracking/trackFindingCDC/topology/EStereoKind.h>
#include <tracking/trackFindingCDC/topology/ISuperLayer.h>
#include <tracking/trackFindingCDC/topology/ILayer.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/numerics/Index.h>

#include <tracking/trackFindingCDC/utilities/FunctorTag.h>

#include <cdc/dataobjects/WireID.h>

#include <utility>
#include <iosfwd>
#include <cassert>

namespace Belle2 {
  class CDCHit;
  namespace CDC {
    class TDCCountTranslatorBase;
    class ADCCountTranslatorBase;
  }
  namespace TrackFindingCDC {
    class CDCTrajectory2D;
    class CDCWireSuperLayer;
    class CDCWire; // IWYU pragma: keep
    class Circle2D;
    class Vector3D;
    class Vector2D;

    /**
     *  Class representing a hit wire in the central drift chamber.
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

      /// Return an instance of the used ADC count translator.
      static CDC::ADCCountTranslatorBase& getADCCountTranslator();

      /// A default value for the drift length variance if no variance from the drift length translation is available.
      static constexpr const double c_simpleDriftLengthVariance = 0.000169;

      /// Default constructor for ROOT compatibility.
      CDCWireHit() = default;

      /**
       *  Constructor associating the CDCHit with estimates of the drift length and charge deposit.
       *
       *  Also stores the CDCHit necessary for later translation to Genfit.
       *
       *  @param ptrHit               Reference to the CDCHit.
       *  @param driftLength          Initial estimate of the drift length at the reference position.
       *  @param driftLengthVariance  Initial estimate of the variance of the dirft length at the reference position.
       *  @param chargeDeposit        Initial estimate of the deposited charge in the drift cell.
       *  @param driftTime            Measured drift time
       */
      CDCWireHit(const CDCHit* ptrHit,
                 double driftLength,
                 double driftLengthVariance = c_simpleDriftLengthVariance,
                 double chargeDeposit = 0,
                 double driftTime = 0);

      /**
       *  Constructor for augmenting the CDCHit with the geometry information of the CDCWire.
       *  Binds to the CDCHit and the corresponding wire together and translates the TDC count to a driftlength.
       *  Also stores the index of the StoreArray
       *  from which the hit has been taken. Necessary for later translation to genfit.
       *
       *  @param  ptrHit          Reference to the CDCHit.
       */
      CDCWireHit(const CDCHit* ptrHit,
                 CDC::TDCCountTranslatorBase* ptrTDCCountTranslator = nullptr,
                 CDC::ADCCountTranslatorBase* ptrADCCountTranslator = nullptr);

      /// Constructor that takes a wire ID and a driftlength at the reference. For testing only!
      CDCWireHit(const WireID& wireID,
                 double driftLength,
                 double driftLengthVariance = c_simpleDriftLengthVariance,
                 double chargeDeposit = 0);

      /// Equality comparison based on the wire and the hit id.
      bool operator==(const CDCWireHit& rhs) const
      {
        return getWireID() == rhs.getWireID() and getRefDriftLength() == rhs.getRefDriftLength();
      }

      /// Total ordering relation based on the wire and the hit id.
      bool operator<(const CDCWireHit& rhs) const
      {
        return
          std::make_pair(getWireID().getEWire(), getRefDriftLength()) <
          std::make_pair(rhs.getWireID().getEWire(), rhs.getRefDriftLength());
      }

      /// Defines CDCWires and CDCWireHits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCWireHit& wireHit, const CDCWire& wire)
      {
        return &wireHit.getWire() < &wire;
      }

      /// Defines CDCWires and CDCWireHits to be coaligned on the wire on which they are based.
      // Same as above but the other way round.
      friend bool operator<(const CDCWire& wire, const CDCWireHit& wireHit)
      {
        return &wire < &wireHit.getWire();
      }

      /// Defines CDCWires and CDCWireHits to be coaligned on the wire on which they are based.
      // Same as above but with CDCWireHit as a pointer.
      friend bool operator<(const CDCWireHit* wireHit, const CDCWire& wire)
      {
        assert(wireHit);
        return *wireHit < wire;
      }

      /// Defines CDCWires and CDCWireHits to be coaligned on the wire on which they are based.
      // Same as above but the other way round and with CDCWireHit as a pointer.
      friend bool operator<(const CDCWire& wire, const CDCWireHit* wireHit)
      {
        assert(wireHit);
        return wire < *wireHit;
      }

      /// Defines CDCWireHits and raw CDCHit to be coaligned.
      bool operator<(const CDCHit& hit);

      friend bool operator<(const CDCWireHit& wireHit, const CDCHit& hit);

      /// Defines wire hits and raw CDCHit to be coaligned.
      // Same as above but the other way round.
      friend bool operator<(const CDCHit& hit, const CDCWireHit& wireHit);

      /// Getter for the CDCHit pointer into the StoreArray.
      const CDCHit* getHit() const
      {
        return m_hit;
      }

      /// Getter for the index of the hit in the StoreArray holding this hit.
      Index getStoreIHit() const;

      /// Getter for the CDCWire the hit is located on.
      const CDCWire& getWire() const
      {
        // if (not m_wire) return attachWire();
        if (not m_wire) return attachWire();
        return *m_wire;
      }

      /**
       *  Reestablishes the pointer of the hit to the wire and returns it
       *  Since the DataStore only transport the event data and not
       *  "static" geometry information the wire is lost whenever the
       *  DataStore is stream across an interprocess boundary or to file.
       *  In this case this method can be called to lookup the wire again.
       */
      const CDCWire& attachWire() const;

      /// Getter for the WireID of the wire the hit is located on.
      const WireID& getWireID() const
      {
        return m_wireID;
      }

      /// Getter for the stereo type of the underlying wire.
      EStereoKind getStereoKind() const
      {
        return ISuperLayerUtil::getStereoKind(getISuperLayer());
      }

      /// Indicator if the underlying wire is axial.
      bool isAxial() const
      {
        return ISuperLayerUtil::isAxial(getISuperLayer());
      }

      /// Getter for the super layer id.
      ISuperLayer getISuperLayer() const
      {
        return getWireID().getISuperLayer();
      }

      /// Getter for the layer id.
      ILayer getILayer() const
      {
        return getWireID().getILayer();
      }

      /// The two dimensional reference position (z=0) of the underlying wire.
      const Vector2D& getRefPos2D() const;

      /// The three dimensional reference position of the underlying wire.
      const Vector3D& getRefPos3D() const;

      /// The distance from the beam line at reference position of the underlying wire.
      double getRefCylindricalR() const;

      /// Getter for the drift length at the reference position of the wire.
      double getRefDriftLength() const
      {
        return m_refDriftLength;
      }

      /// Getter for the variance of the drift length at the reference position of the wire.
      double getRefDriftLengthVariance() const
      {
        return m_refDriftLengthVariance;
      }

      /// Getter for the charge due to energy deposit in the drift cell
      double getRefChargeDeposit() const
      {
        return m_refChargeDeposit;
      }

      /// Checks if the wire hit is based on the given wire.
      bool isOnWire(const CDCWire& wire) const
      {
        return &getWire() == &wire;
      }

      /**
       *  Reconstructs a position of primary ionisation on the drift circle.
       *
       *  The result is the position of closest approach on the drift circle to the trajectory.
       *
       *  All positions and the trajectory are interpreted to lie at z=0.
       *  Also the right left passage hypotheses does not play a role in
       *  the reconstruction in any way.
       */
      Vector2D reconstruct2D(const CDCTrajectory2D& trajectory2D) const;

      /**
       *  Attempts to reconstruct a three dimensional position (especially of stereo hits).
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
      Vector3D reconstruct3D(const CDCTrajectory2D& trajectory2D, ERightLeft rlInfo, double z = 0) const;

      /**
       *  Applys the conformal transformation to the drift circle this hit represents.
       *
       *  Although not advisable to use we provide an interface to obtain the inversion
       *  at the unit circle as a transformation to the so called conformal space.
       *  We provide the self-inverse for of the transformation
       *  - \f$X = x / (x^2 + y^2 - r^2)\f$
       *  - \f$Y = y / (x^2 + y^2 - r^2)\f$
       *  - \f$R = r / (x^2 + y^2 - r^2)\f$
       */
      Circle2D conformalTransformed(const Vector2D& relativeTo) const;

      /// Mutable getter for the automaton cell.
      AutomatonCell& getAutomatonCell() const
      {
        return m_automatonCell;
      }

      /// Indirection to the automaton cell for easier access to the flags
      AutomatonCell* operator->() const
      {
        return &m_automatonCell;
      }

      /// Getter for the super cluster id
      int getISuperCluster() const
      {
        return m_iSuperCluster;
      }

      /// Setter for the super cluster id
      void setISuperCluster(int iSuperCluster)
      {
        m_iSuperCluster = iSuperCluster;
      }

      /// Return the drift time measured by the CDC for this hit
      double getDriftTime() const
      {
        return m_refDriftTime;
      }

    private:
      /// Memory for the WireID.
      WireID m_wireID;

      /// Memory for the CDCWire pointer - Trailing comment indicates to not stream this member
      mutable CDCWire const* m_wire = nullptr; //!

      /// Memory for the automaton cell.
      mutable AutomatonCell m_automatonCell{1};

      /// Memory for the drift length at the wire reference point.
      double m_refDriftLength = 0;

      /// Memory for the variance of the drift length at the wire reference point.
      double m_refDriftLengthVariance = c_simpleDriftLengthVariance;

      /// Memory for the charge induced by the energy deposit in the drift cell.
      double m_refChargeDeposit = 0.0;

      /// Memory for the super cluster id
      int m_iSuperCluster = -1;

      /// Measured drift time of the CDC hit
      double m_refDriftTime = 0.0f;

      /// Memory for the CDCHit pointer.
      const CDCHit* m_hit = nullptr;
    };

    /// Generic functor to get the wire hit from an object.
    struct GetWireHit {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Returns the wire hit of an object.
      template<class T, class SFINAE = decltype(&T::getWireHit)>
      const CDCWireHit & operator()(const T& t) const
      {
        return t.getWireHit();
      }

      /// If given a wire hit return it unchanged.
      const CDCWireHit& operator()(const CDCWireHit& wireHit) const
      {
        return wireHit;
      }
    };

    /// String output operator for wire hit objects to help debugging.
    std::ostream& operator<<(std::ostream& output, const CDCWireHit& wirehit);

    /// Defines CDCWireHits and raw CDCHit to be coaligned.
    bool operator<(const CDCWireHit& wireHit, const CDCHit& hit);

    /// Defines CDCWireHits and raw CDCHit to be coaligned.
    bool operator<(const CDCHit& hit, const CDCWireHit& wireHit);;

    /// Defines CDCWireSuperLayer and CDCWireHit to be coordered with the super layers
    bool operator<(const CDCWireHit& wireHit, const CDCWireSuperLayer& wireSuperLayer);

    /// Defines CDCWireSuperLayer and CDCWireHit to be coordered with the super layers
    // Same as above but the other way round.
    bool operator<(const CDCWireSuperLayer& wireSuperLayer, const CDCWireHit& wireHit);
  }
}
