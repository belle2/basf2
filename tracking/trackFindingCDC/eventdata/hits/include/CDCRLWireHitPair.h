/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/topology/EStereoKind.h>
#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;
    class CDCWire;
    class WireNeighborKind;

    /// A pair of oriented wire hits.
    class CDCRLWireHitPair {
    public:
      /// Default constructor for ROOT
      CDCRLWireHitPair() = default;

      /// Constructor taking two oriented wire hits.
      CDCRLWireHitPair(const CDCRLWireHit& fromRLWireHit,
                       const CDCRLWireHit& toRLWireHit,
                       int iCluster = 0);

      /// Constructs a oriented wire hit pair that is the reverse of this one.
      CDCRLWireHitPair reversed() const;

      /// Reverses the oriented wire hit pair inplace.
      void reverse();

      /// Returns the aliased version of this oriented wire hit pair
      CDCRLWireHitPair getAlias() const;

      /// Equality comparision based the two oriented wire hits.
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
      friend bool operator<(const CDCRLWireHitPair& rlWireHitPair, const CDCWireHit& wireHit)
      {
        return rlWireHitPair.getFromRLWireHit() < wireHit;
      }

      /// Defines wire hits and oriented wire hit pair as coaligned on the first wire hit.
      friend bool operator<(const CDCWireHit& wireHit, const CDCRLWireHitPair& rlWireHitPair)
      {
        return wireHit < rlWireHitPair.getFromRLWireHit();
      }

      /// Getter for the WireNeighborKind of the two underlying wires
      WireNeighborKind getNeighborKind() const;

      /// Getter for the common superlayer id of the pair.
      ISuperLayer getISuperLayer() const
      {
        ISuperLayer result = getFromRLWireHit().getISuperLayer();
        return result == getToRLWireHit().getISuperLayer() ? result : ISuperLayerUtil::c_Invalid;
      }

      /// Getter for the common stereo type of the superlayer of the pair.
      EStereoKind getStereoKind() const
      {
        return ISuperLayerUtil::getStereoKind(getISuperLayer());
      }

      /// Indicator if the underlying wires are axial.
      bool isAxial() const
      {
        return getStereoKind() == EStereoKind::c_Axial;
      }

      /// Getter for the wire the first oriented wire hit is based on.
      const CDCWire& getFromWire() const
      {
        return getFromRLWireHit().getWire();
      }

      /// Getter for the wire the second oriented wire hit is based on.
      const CDCWire& getToWire() const
      {
        return getToRLWireHit().getWire();
      }

      /// Indicator if any of the two oriented wire hits is based on the given wire.
      bool hasWire(const CDCWire& wire) const
      {
        return getFromRLWireHit().isOnWire(wire) or getToRLWireHit().isOnWire(wire);
      }

      /// Getter for the hit wire of the first oriented wire hit.
      const CDCWireHit& getFromWireHit() const
      {
        return getFromRLWireHit().getWireHit();
      }

      /// Getter for the hit wire of the second oriented wire hit.
      const CDCWireHit& getToWireHit() const
      {
        return getToRLWireHit().getWireHit();
      }

      /// Indicator if any of the two oriented wire hits is based on the given wire hit.
      bool hasWireHit(const CDCWireHit& wirehit) const
      {
        return getFromRLWireHit().hasWireHit(wirehit) or getToRLWireHit().hasWireHit(wirehit);
      }

      /// Getter for the right left passage information of the first oriented wire hit.
      ERightLeft getFromRLInfo() const
      {
        return getFromRLWireHit().getRLInfo();
      }

      /// Getter for the right left passage information of the second oriented wire hit.
      ERightLeft getToRLInfo() const
      {
        return getToRLWireHit().getRLInfo();
      }

      /// Getter for the  first oriented wire hit.
      CDCRLWireHit& getFromRLWireHit()
      {
        return m_fromRLWireHit;
      }

      /// Getter for the  second oriented wire hit.
      CDCRLWireHit& getToRLWireHit()
      {
        return m_toRLWireHit;
      }

      /// Constant getter for the  first oriented wire hit.
      const CDCRLWireHit& getFromRLWireHit() const
      {
        return m_fromRLWireHit;
      }

      /// Constant getter for the  second oriented wire hit.
      const CDCRLWireHit& getToRLWireHit() const
      {
        return m_toRLWireHit;
      }

      /// Setter for the first oriented wire hit.
      void setFromRLWireHit(const CDCRLWireHit& fromRLWireHit)
      {
        m_fromRLWireHit = fromRLWireHit;
      }

      /// Setter for the second oriented wire hit.
      void setToRLWireHit(const CDCRLWireHit& toRLWireHit)
      {
        m_toRLWireHit = toRLWireHit;
      }

      /// Setter for the right left passage information of the first oriented wire hit.
      void setFromRLInfo(ERightLeft fromRLInfo);

      /// Setter for the right left passage information of the second oriented wire hit.
      void setToRLInfo(ERightLeft toRLInfo);

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
      CDCRLWireHit m_fromRLWireHit;

      /// Memory for the reference to the second oriented wire hit.
      CDCRLWireHit m_toRLWireHit;

      /// Memory for the cluster id of this facet
      int m_iCluster = -1;
    };
  }
}
