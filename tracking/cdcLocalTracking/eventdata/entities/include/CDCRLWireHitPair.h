/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCRLWIREHITPAIR_H
#define CDCRLWIREHITPAIR_H

#include "CDCRLWireHit.h"


namespace Belle2 {
  namespace CDCLocalTracking {

    /// A pair of oriented wire hits
    class CDCRLWireHitPair : public UsedTObject {

    public:

      /// Default constructor for ROOT compatibility.
      CDCRLWireHitPair();

      /// Copy constructor applying checks
      CDCRLWireHitPair(const CDCRLWireHitPair& rlWireHitPair);

      /// Constructor taking two oriented wire hits
      CDCRLWireHitPair(const CDCRLWireHit* fromRLWireHit, const CDCRLWireHit* toRLWireHit);

      /// Empty deconstructor
      ~CDCRLWireHitPair();



      /// Constructs a oriented wire hit pair that is the reverse of this one
      CDCRLWireHitPair reversed() const;

      /// Reverses the oriented wire hit pair inplace.
      void reverse();



      /// Equality comparision based the two oriented wire hits.
      bool operator==(const CDCRLWireHitPair& other) const
      { return getFromRLWireHit() == other.getFromRLWireHit() and getToRLWireHit() == other.getToRLWireHit(); }

      /// Total ordering relation based on the from oriented wire hit first and the to oriented wire hit second
      bool operator<(const CDCRLWireHitPair& other) const {
        return getFromRLWireHit() < other.getFromRLWireHit() or
               (getFromRLWireHit() == other.getFromRLWireHit() and getToRLWireHit() < other.getToRLWireHit());
      }



      /// Defines wire hits and oriented wire hit pair as coaligned on the first wire hit
      friend bool operator<(const  CDCRLWireHitPair& rlWireHitPair, const CDCWireHit& wireHit) { return rlWireHitPair.getFromWireHit() < wireHit; }

      /// Defines wire hits and oriented wire hit pair as coaligned on the first wire hit
      friend bool operator<(const CDCWireHit& wireHit, const CDCRLWireHitPair& rlWireHitPair) { return  wireHit <  rlWireHitPair.getFromWireHit(); }



      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference there is no way to tell \n
       *  if one should use the dot '.' or operator '->' for method look up. \n
       *  So this function defines the -> operator for the object. \n
       *  No matter you have a pointer or an object access is given with '->'*/
      const CDCRLWireHitPair* operator->() const { return this; }



      /// Estimate the transvers travel distance on the given circle to the first oriented wire hit
      FloatType getFrontPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getFromRLWireHit().getFrontPerpS(trajectory2D); }

      /// Estimate the transvers travel distance on the given circle to the second oriented wire hit
      FloatType getBackPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getToRLWireHit().getBackPerpS(trajectory2D); }

      /// Calculates the squared distance of the two oriented wire hit to a circle as see from the transvers plane.
      /** Returns the sum of the squared distances to the two oriented wire hits */
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const
      { return getFromRLWireHit().getSquaredDist2D(trajectory2D) + getToRLWireHit().getSquaredDist2D(trajectory2D);}



      /// Getter for the WireNeighborType of the two underlying wires - zero for none neighbors
      WireNeighborType getNeighborType() const { return getFromWire().isNeighborWith(getToWire()); }



      /// Getter for the common superlayer id of the pair
      ILayerType getISuperLayer() const {
        ILayerType result = getFromRLWireHit().getISuperLayer();
        return result == getToRLWireHit().getISuperLayer() ? result : INVALIDSUPERLAYER;
      }

      /// Getter for the common stereo type of the superlayer of the pair
      StereoType getStereoType() const
      { return isValidISuperLayer(getISuperLayer()) ? getFromRLWireHit().getStereoType() : INVALID_STEREOTYPE; }

      /// Getter for the wire the first oriented wire hit is based on
      const CDCWire& getFromWire() const { return getFromRLWireHit()->getWire(); }

      /// Getter for the wire the second oriented wire hit is based on
      const CDCWire& getToWire() const { return getToRLWireHit()->getWire(); }

      /// Indicator if any of the two oriented wire hits is based on the given wire
      bool hasWire(const CDCWire& wire) const
      { return getFromRLWireHit()->hasWire(wire) or getToRLWireHit()->hasWire(wire); }



      /// Getter for the hit wire of the first oriented wire hit
      const CDCWireHit& getFromWireHit() const { return getFromRLWireHit()->getWireHit(); }

      /// Getter for the hit wire of the second oriented wire hit
      const CDCWireHit& getToWireHit() const { return getToRLWireHit()->getWireHit(); }

      /// Indicator if any of the two oriented wire hits is based on the given wire hit
      bool hasWireHit(const CDCWireHit& wirehit) const
      { return getFromRLWireHit()->hasWireHit(wirehit) or getToRLWireHit()->hasWireHit(wirehit); }

      /// Returns the wirehit *this* oriented wire hit pair and the given oriented wire hit pair have in common. Nullptr for no common wire hit.
      const CDCWireHit* commonWireHit(const CDCRLWireHitPair& rlWireHitPair) const;



      /// Getter for the right left passage information of the first oriented wire hit
      const RightLeftInfo& getFromRLInfo() const { return getFromRLWireHit()->getRLInfo(); }

      /// Getter for the right left passage information of the second oriented wire hit
      const RightLeftInfo& getToRLInfo() const { return getToRLWireHit()->getRLInfo(); }



      /// Getter for the  first oriented wire hit
      const CDCRLWireHit& getFromRLWireHit() const { return *m_fromRLWireHit; }

      /// Getter for the  second oriented wire hit
      const CDCRLWireHit& getToRLWireHit() const { return *m_toRLWireHit; }



      /// Setter for the first oriented wire hit
      void setFromRLWireHit(const CDCRLWireHit* fromRLWireHit)
      { m_fromRLWireHit = fromRLWireHit; }

      /// Setter for the second oriented wire hit
      void setToRLWireHit(const CDCRLWireHit* toRLWireHit)
      { m_toRLWireHit = toRLWireHit; }


    public:
      /// Setter for the right left passage information of the first oriented wire hit
      void setFromRLInfo(const RightLeftInfo& fromRLInfo);

      /// Setter for the right left passage information of the second oriented wire hit
      void setToRLInfo(const RightLeftInfo& toRLInfo);

    protected:
      const CDCRLWireHit* m_fromRLWireHit; ///< Memory for the reference to the first oriented wire hit
      const CDCRLWireHit* m_toRLWireHit;   ///< Memory for the reference to the second oriented wire hit

      /// ROOT Macro to make CDCRLWireHitPair a ROOT class.
      ClassDefInCDCLocalTracking(CDCRLWireHitPair, 1);

    }; //end class CDCRLWireHitPair
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //CDCRLWIREHITPAIR_H

