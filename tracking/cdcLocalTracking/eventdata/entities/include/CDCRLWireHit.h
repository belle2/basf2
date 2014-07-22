/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRLWIREHIT_H
#define CDCRLWIREHIT_H


#include <cdc/dataobjects/CDCSimHit.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include "CDCWireHit.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing an oriented hit wire including a hypotheses whether the causing track passes left or right
    class CDCRLWireHit : public UsedTObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRLWireHit();

      /// Constructs an oriented wire hit
      /** Constructs an oriented wire hit
       *  @param wireHit the wire hit the oriented hit is assoziated with.
       *  @param rlInfo the right left passage information the _wire_ relativ to the track */
      CDCRLWireHit(const CDCWireHit* wireHit, RightLeftInfo rlInfo = 0);

      /// Empty deconstructor
      ~CDCRLWireHit();

      /// Constructs an oriented wire hit from a sim hit and the assoziated wirehit.
      /** This translates the sim hit to an oriented wire hit mainly to be able to compare the \n
       *  reconstructed values from the algorithm with the Monte Carlo information. \n
       *  It merely evalutates, if the true trajectory passes right or left of the wire. */
      static CDCRLWireHit fromSimHit(const CDCWireHit* wirehit, const CDCSimHit& simhit);

      /// Returns the oriented wire hit with the opposite right left information.
      const CDCRLWireHit* reversed() const;

      /// Make the wire hit automatically castable to its underlying cdcHit
      operator const Belle2::CDCHit* () const
      { return (const CDCHit*) getWireHit(); }


      /// Equality comparision based on wire hit, left right passage information.
      bool operator==(const CDCRLWireHit& other) const {
        return getWireHit() == other.getWireHit() and getRLInfo() == other.getRLInfo();
      }

      /// Total ordering relation based on wire hit and left right passage information in this order of importance.
      bool operator<(const CDCRLWireHit& other) const {
        return getWireHit() <  other.getWireHit() or (
                 getWireHit() == other.getWireHit() and (
                   getRLInfo() < other.getRLInfo()));
      }

      /// Defines wires and oriented wire hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCRLWireHit& rlWireHit, const CDCWire& wire) { return rlWireHit.getWire() < wire; }

      /// Defines wires and oriented wire hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCWire& wire, const CDCRLWireHit& rlWireHit) { return wire < rlWireHit.getWire(); }

      /// Defines wire hits and oriented wire hits to be coaligned on the wire hit on which they are based.
      friend bool operator<(const CDCRLWireHit& rlWireHit, const CDCWireHit& wireHit) { return rlWireHit.getWireHit() < wireHit; }

      /// Defines wire hits and oriented wire hits to be coaligned on the wire hit on which they are based.
      friend bool operator<(const CDCWireHit& wireHit, const CDCRLWireHit& rlWireHit) { return wireHit < rlWireHit.getWireHit(); }



      /// The two dimensional reference position of the underlying wire
      const Vector2D& getRefPos2D() const { return getWireHit().getRefPos2D(); }

      /// Getter for the wire the oriented hit assoziated to.
      const CDCWire& getWire() const { return getWireHit().getWire(); }

      /// Getter for the  drift length at the reference position of the wire
      FloatType getRefDriftLength() const
      { return getWireHit().getRefDriftLength(); }

      /// Getter for the  drift length at the reference position of the wire
      FloatType getSignedRefDriftLength() const
      { return ((SignType)(getRLInfo())) * getRefDriftLength(); }

      /// Getter for the variance of the drift length at the reference position of the wire.
      FloatType getRefDriftLengthVariance() const
      { return getWireHit().getRefDriftLengthVariance(); }

      /// Getter for the wire hit assoziated with the oriented hit.
      const CDCWireHit& getWireHit() const { return *m_wirehit; }

      /// Getter for the right left passage information.
      const RightLeftInfo& getRLInfo() const { return m_rlInfo; }

      /// Estimate the transvers travel distance on the given circle.
      /** Uses the point of closest approach to the wire hit position
       *  on the circle and calculates the arc length from the reference on the circle.
       *  @return The arc length on the circle from the reference */
      FloatType getPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getWireHit().getPerpS(trajectory2D); }


      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference there is no way to tell \n
       *  if one should use the dot '.' or operator '->' for method look up. \n
       *  So this function defines the -> operator for the object. \n
       *  No matter you have a pointer or an object access is given with '->'*/
      const CDCRLWireHit* operator->() const { return this; }

      /** @name Methods common to all tracking entities
       *  All entities ( track parts contained in a single superlayer ) share this interface to help definition of collections of them.
       */
      /**@{*/
      /// Checks if the oriented hit is assoziated with the give wire
      bool hasWire(const CDCWire& wire) const
      { return getWireHit().getWire() == wire; }

      /// Checks if the oriented hit is assoziated with the give wire hit
      bool hasWireHit(const CDCWireHit& wirehit) const
      { return getWireHit() == wirehit; }

      /// Center of mass is the refernce position for underlying wire hits.
      Vector2D getCenterOfMass2D() const
      { return getWireHit().getRefPos2D(); }

      /// Getter for the stereo type of the underlying wire.
      StereoType getStereoType() const
      { return getWireHit().getStereoType(); }

      /// Getter for the superlayer id
      ILayerType getISuperLayer() const
      { return getWireHit().getISuperLayer(); }

      /// Same as getPerpS().
      FloatType getFrontPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Same as getPerpS().
      FloatType getBackPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Calculates the squared distance of the oriented hit to a circle as see from the transvers plane.
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const {
        FloatType correctedDistance = trajectory2D.getDist2D(getRefPos2D()) - getRLInfo() * getRefDriftLength();
        return correctedDistance * correctedDistance;
      }

      /// Output operator. Help debugging.
      friend std::ostream& operator<<(std::ostream& output, const CDCRLWireHit& rlWireHit) {
        output << "CDCRLWireHit(" << rlWireHit.getWireHit() << ","
               << rlWireHit.getRLInfo() << ")" ;
        return output;
      }

    private:

      const CDCWireHit* m_wirehit;  ///< Memory for the reference to the assiziated wire hit
      RightLeftInfo m_rlInfo; ///< Memory for the right left passage information of the oriented wire hit.

      /// ROOT Macro to make CDCRLWireHit a ROOT class.
      ClassDefInCDCLocalTracking(CDCRLWireHit, 1);

    }; //class CDCRLWireHit

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRLWIREHIT_H
