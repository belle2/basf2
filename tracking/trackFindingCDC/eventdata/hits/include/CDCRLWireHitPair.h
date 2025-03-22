/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/eventdata/hits/CDCRLWireHit.h>

#include <tracking/trackingUtilities/numerics/ERightLeft.h>
#include <tracking/trackingUtilities/topology/EStereoKind.h>
#include <tracking/trackingUtilities/topology/ISuperLayer.h>

namespace Belle2 {

  namespace TrackingUtilities {
    class CDCWireHit;
    class CDCWire;
    class WireNeighborKind;
  }
  namespace TrackFindingCDC {

    /// A pair of oriented wire hits.
    class CDCRLWireHitPair {
    public:
      /// Default constructor for ROOT
      CDCRLWireHitPair() = default;

      /// Constructor taking two oriented wire hits.
      CDCRLWireHitPair(const TrackingUtilities::CDCRLWireHit& fromRLWireHit,
                       const TrackingUtilities::CDCRLWireHit& toRLWireHit,
                       int iCluster = 0);

      /// Constructs a oriented wire hit pair that is the reverse of this one.
      CDCRLWireHitPair reversed() const;

      /// Reverses the oriented wire hit pair inplace.
      void reverse();

      /// Returns the aliased version of this oriented wire hit pair
      CDCRLWireHitPair getAlias() const;

      /// Equality comparison based the two oriented wire hits.
      bool operator==(const CDCRLWireHitPair& other) const
      {
        return getFromRLWireHit() == other.getFromRLWireHit() and
               getToRLWireHit() == other.getToRLWireHit();
      }

      /** Total ordering relation based on
       *  the from oriented wire hit first and
       *  the to oriented wire hit second.
       */
      bool operator<(const CDCRLWireHitPair& other) const
      {
        return getICluster() < other.getICluster() or
               (getICluster() == other.getICluster() and
                (getFromRLWireHit() < other.getFromRLWireHit() or
                 (getFromRLWireHit() == other.getFromRLWireHit() and
                  getToRLWireHit() < other.getToRLWireHit())));
      }

      /// Defines wire hits and oriented wire hit pair as coaligned on the first wire hit.
      friend bool operator<(const CDCRLWireHitPair& rlWireHitPair, const TrackingUtilities::CDCWireHit& wireHit)
      {
        return rlWireHitPair.getFromRLWireHit() < wireHit;
      }

      /// Defines wire hits and oriented wire hit pair as coaligned on the first wire hit.
      friend bool operator<(const TrackingUtilities::CDCWireHit& wireHit, const CDCRLWireHitPair& rlWireHitPair)
      {
        return wireHit < rlWireHitPair.getFromRLWireHit();
      }

      /// Getter for the WireNeighborKind of the two underlying wires
      TrackingUtilities::WireNeighborKind getNeighborKind() const;

      /// Getter for the common superlayer id of the pair.
      TrackingUtilities::ISuperLayer getISuperLayer() const
      {
        TrackingUtilities::ISuperLayer result = getFromRLWireHit().getISuperLayer();
        return result == getToRLWireHit().getISuperLayer() ? result : TrackingUtilities::ISuperLayerUtil::c_Invalid;
      }

      /// Getter for the common stereo type of the superlayer of the pair.
      TrackingUtilities::EStereoKind getStereoKind() const
      {
        return TrackingUtilities::ISuperLayerUtil::getStereoKind(getISuperLayer());
      }

      /// Indicator if the underlying wires are axial.
      bool isAxial() const
      {
        return getStereoKind() == TrackingUtilities::EStereoKind::c_Axial;
      }

      /// Getter for the wire the first oriented wire hit is based on.
      const TrackingUtilities::CDCWire& getFromWire() const
      {
        return getFromRLWireHit().getWire();
      }

      /// Getter for the wire the second oriented wire hit is based on.
      const TrackingUtilities::CDCWire& getToWire() const
      {
        return getToRLWireHit().getWire();
      }

      /// Indicator if any of the two oriented wire hits is based on the given wire.
      bool hasWire(const TrackingUtilities::CDCWire& wire) const
      {
        return getFromRLWireHit().isOnWire(wire) or getToRLWireHit().isOnWire(wire);
      }

      /// Getter for the hit wire of the first oriented wire hit.
      const TrackingUtilities::CDCWireHit& getFromWireHit() const
      {
        return getFromRLWireHit().getWireHit();
      }

      /// Getter for the hit wire of the second oriented wire hit.
      const TrackingUtilities::CDCWireHit& getToWireHit() const
      {
        return getToRLWireHit().getWireHit();
      }

      /// Indicator if any of the two oriented wire hits is based on the given wire hit.
      bool hasWireHit(const TrackingUtilities::CDCWireHit& wirehit) const
      {
        return getFromRLWireHit().hasWireHit(wirehit) or getToRLWireHit().hasWireHit(wirehit);
      }

      /// Getter for the right left passage information of the first oriented wire hit.
      TrackingUtilities::ERightLeft getFromRLInfo() const
      {
        return getFromRLWireHit().getRLInfo();
      }

      /// Getter for the right left passage information of the second oriented wire hit.
      TrackingUtilities::ERightLeft getToRLInfo() const
      {
        return getToRLWireHit().getRLInfo();
      }

      /// Getter for the  first oriented wire hit.
      TrackingUtilities::CDCRLWireHit& getFromRLWireHit()
      {
        return m_fromRLWireHit;
      }

      /// Getter for the  second oriented wire hit.
      TrackingUtilities::CDCRLWireHit& getToRLWireHit()
      {
        return m_toRLWireHit;
      }

      /// Constant getter for the  first oriented wire hit.
      const TrackingUtilities::CDCRLWireHit& getFromRLWireHit() const
      {
        return m_fromRLWireHit;
      }

      /// Constant getter for the  second oriented wire hit.
      const TrackingUtilities::CDCRLWireHit& getToRLWireHit() const
      {
        return m_toRLWireHit;
      }

      /// Setter for the first oriented wire hit.
      void setFromRLWireHit(const TrackingUtilities::CDCRLWireHit& fromRLWireHit)
      {
        m_fromRLWireHit = fromRLWireHit;
      }

      /// Setter for the second oriented wire hit.
      void setToRLWireHit(const TrackingUtilities::CDCRLWireHit& toRLWireHit)
      {
        m_toRLWireHit = toRLWireHit;
      }

      /// Setter for the right left passage information of the first oriented wire hit.
      void setFromRLInfo(TrackingUtilities::ERightLeft fromRLInfo);

      /// Setter for the right left passage information of the second oriented wire hit.
      void setToRLInfo(TrackingUtilities::ERightLeft toRLInfo);

      /// Getter for the cluster id
      int getICluster() const
      {
        return m_iCluster;
      }

      /// Setter for the cluster id
      void setICluster(int iCluster)
      {
        m_iCluster = iCluster;
      }

    protected:
      /// Memory for the reference to the first oriented wire hit.
      TrackingUtilities::CDCRLWireHit m_fromRLWireHit;

      /// Memory for the reference to the second oriented wire hit.
      TrackingUtilities::CDCRLWireHit m_toRLWireHit;

      /// Memory for the cluster id of this facet
      int m_iCluster = -1;
    };
  }
}
