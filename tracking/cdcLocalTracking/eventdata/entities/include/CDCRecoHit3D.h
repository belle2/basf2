/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOHIT3D_H
#define CDCRECOHIT3D_H

#include <cmath>

#include <cdc/dataobjects/CDCSimHit.h>

//#include <TVector3.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/trajectories/CDCTrajectorySZ.h>

#include "CDCWireHit.h"
#include "CDCRLWireHit.h"
#include "CDCRecoHit2D.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing a three dimensional reconstructed hit.
    /** A recohit represents a likely point where the particle went through. It is always assoziated with a \n
     *  wire hit it seeks to reconstruct. The reconstructed point is stored as an absolut position from the \n
     *  origin / interaction point. In addition the reconstructed hit takes a right left passage information \n
     *  which indicates if the hit wire lies to the right or to the left of the particle trajectory causing the hit. \n
     *  The later readily indicates a flight direction from the reconstructed hit, if a tangential approch of \n
     *  the trajectory to the drift circle is assumed.

     *  Finally we want to estimate the travel distance to z coordinate relation of the particle trajectory. \n
     *  Therefore the three dimensional reconstructed hit stores the travel distance as seen from the xy projection \n
     *  ( perpS ) it took to get to hit. This variable can be calculated from the trajectory circle fitted in \n
     *  the two dimensional tracking as the arc length.
     */

    class CDCRecoHit3D : public CDCLocalTracking::UsedTObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoHit3D();

      /// Constructor taking all stored variables of the reconstructed hit.
      CDCRecoHit3D(const CDCRLWireHit* rlWireHit,
                   const Vector3D& position,
                   FloatType perpS = 0);

    public:

      /// Constructs a three dimensional reconstructed hit from a sim hit and the assoziated wirehit.
      /** This translates the sim hit to a reconstructed hit mainly to be able to compare the \n
       *  reconstructed values from the algorithm with the Monte Carlo information. \n
       *  It merely takes the position from the sim hit and calculates the right left passage information. \n
       *  Since only the time is present in the sim hit but not the travel distance this parameter is just set
       *  NAN!
       * */
      static CDCRecoHit3D fromSimHit(const CDCWireHit* wireHit, const CDCSimHit& simHit);

      /// Reconstructs the three dimensional hit from the two dimensional and the two dimensional trajectory.
      /** For two dimensional reconstructed hits on axial wires this reconstructs \n
       *  the xy position and the transvers travel distance. The z coordinate cannot \n
       *  be determined since the projection can not show any information about it. It
       *  is always set to NAN for axial hits.
       *
       *  For two dimensional reconstructed hits on stereo wires however the deviation of the xy position \n
       *  can be used to get z information. The reconstucted hit lies exactly on the fitted trajectory \n
       *  as seen from the xy plane. Hence also xy position and transvers travel distance are available. \n
       *  Only the stereo hits have then the full information to go head and make the sz trajectory. */
      static CDCRecoHit3D reconstruct(const CDCRecoHit2D& recoHit,
                                      const CDCTrajectory2D& trajectory2D);

      /// Reconstructs the three dimensional hit from the wire hit and the given right left passage information by shifting it to a z coordinate, where it touches the two dimensional trajectory from the side indicated by the right left passage.
      static CDCRecoHit3D reconstruct(const CDCRLWireHit& rlWireHit,
                                      const CDCTrajectory2D& trajectory2D);



      /// Reconstructs the three dimensional hit from the two dimensional, the two dimensional trajectory and sz trajectory.
      /** For two dimensional reconstructed hits on axial wires this reconstructs \n
       *  the xy position and the transvers travel distance. The z coordinate is then determined \n
       *  by evaluating the sz trajectory at the just calculated travel distance. Note that it is important \n
       *  that both circle trajectory and sz trajectory share a common reference point.
       *
       *  For two dimensional reconstructed hits on stereo wires the transerse travel distance is obtained \n
       *  as in the reconstuct() method before. However the z coordinate is set to the value of the sz trajectory \n
       *  at just calculated the transvers travel distance, since the trajectory should be more exact than the shifting \n
       *  along the wire.*/
      static CDCRecoHit3D reconstruct(const CDCRecoHit2D& recoHit,
                                      const CDCTrajectory2D& trajectory2D,
                                      const CDCTrajectorySZ& trajectorySZ);

      /// Constructs the average of two reconstructed hit positions. \n
      /** Averages the hit positions and the travel distance. The function averages only reconstructed hits \n
       *  assoziated with the same wire hit. If not all recostructed hits are on the same wire hit, the first hit \n
       *  is returned unchanged. Also averages the right left passage information with averageInfo(). */
      static CDCRecoHit3D average(const CDCRecoHit3D& first,
                                  const CDCRecoHit3D& second);

      /// Empty deconstructor
      ~CDCRecoHit3D();



      /// Turns the orientation in place.
      /** Changes the sign of the right left passage information, since the position remains the same by this reversion.*/
      void reverse();

      /** Returns the recohit with the opposite right left information */
      CDCRecoHit3D reversed() const;



      /// Equality comparision based on wire hit, right left passage information and reconstructed position.
      bool operator==(const CDCRecoHit3D& other) const {
        return getRLWireHit() == other.getRLWireHit() and
               getRLInfo() == other.getRLInfo() and
               getRecoPos3D() == other.getRecoPos3D();
      }



      /// Total ordering relation based on wire hit, right left passage information and position information in this order of importance.
      bool operator<(const CDCRecoHit3D& other) const {
        return getRLWireHit() < other.getRLWireHit() or (
                 getRLWireHit() == other.getRLWireHit() and
                 getRecoPos3D() < other.getRecoPos3D());
      }



      /// Defines wires and the three dimensional reconstructed hits as coaligned
      friend bool operator<(const CDCRecoHit3D& recoHit3D, const CDCWire& wire) { return recoHit3D.getWire() < wire; }

      /// Defines wires and the three dimensional reconstructed hits as coaligned
      friend bool operator<(const CDCWire& wire, const CDCRecoHit3D& recoHit3D) { return wire < recoHit3D.getWire(); }

      /// Defines wire hits and the three dimensional reconstructed hits as coaligned
      friend bool operator<(const CDCRecoHit3D& recoHit3D, const CDCWireHit& wireHit) { return recoHit3D.getWireHit() < wireHit; }

      /// Defines wire hits and the three dimensional reconstructed hits as coaligned
      friend bool operator<(const CDCWireHit& wireHit, const CDCRecoHit3D& recoHit3D) { return wireHit < recoHit3D.getWireHit(); }



      /// Getter for the stereo type of the underlying wire.
      StereoType getStereoType() const
      { return getRLWireHit().getStereoType(); }

      /// Getter for the superlayer id
      ILayerType getISuperLayer() const
      { return getRLWireHit().getISuperLayer(); }



      /// Getter for the wire.
      const CDCWire& getWire() const { return getRLWireHit().getWire(); }

      /// Checks if the reconstructed hit is assoziated with the give wire.
      bool hasWire(const CDCWire& wire) const
      { return getRLWireHit().hasWire(wire); }



      /// Getter for the wire hit
      const CDCWireHit& getWireHit() const { return getRLWireHit().getWireHit(); }

      /// Checks if the reconstructed hit is assoziated with the give wire hit
      bool hasWireHit(const CDCWireHit& wireHit) const
      { return getRLWireHit().hasWireHit(wireHit); }



      /// Getter for the oriented wire hit
      const CDCRLWireHit& getRLWireHit() const { return *m_rlWireHit; }

      /// Setter for the oriented wire hit assoziated with the reconstructed hit.
      void setRLWireHit(const CDCRLWireHit* rlWireHit)
      { m_rlWireHit = rlWireHit; }



      /// Getter for the right left passage information,
      /** Returns the right left passage information as see in the xy projection.
       *  It gives if the wire lies on the right or on the left side of the track \n
       *  as you at the xy projection. */
      RightLeftInfo getRLInfo() const
      { return getRLWireHit().getRLInfo(); }

      /// Getter for the 3d position of the hit
      const Vector3D& getRecoPos3D() const
      { return m_recoPos3D; }

      /// Setter for the 3d position of the hit
      void setRecoPos3D(const Vector3D& recoPos3D)
      { m_recoPos3D = recoPos3D; }

      /// Getter for the 2d position of the hit
      const Vector2D& getRecoPos2D() const { return getRecoPos3D().xy(); }

      /// Getter for the z coordinate of the reconstructed position
      const FloatType& getRecoZ() const { return getRecoPos3D().z(); }

      /// Projects the hit's reconstructed position onto the given two dimensional trajectory
      Vector2D getRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.getClosest(getRecoPos2D()); }

      /// Gets the displacement from the wire position in the xy plain at the reconstructed position
      Vector2D getRecoDisp2D() const;

      /// Constructs a two dimensional reconstructed hit by carrying out the stereo ! projection to the wire reference postion
      CDCRecoHit2D getRecoHit2D() const
      { return CDCRecoHit2D(m_rlWireHit, getRecoDisp2D()); }



      /// Returns the position of the wire in the xy plain the reconstructed position is located in
      Vector2D getRecoWirePos2D() const
      { return getWire().getWirePos2DAtZ(getRecoZ()); }

      /// Returns the drift length next to the reconstructed position. Dummy implemented as the reference drift length
      FloatType getSignedRecoDriftLength() const
      { return getRLWireHit().getSignedRefDriftLength(); }

      /// Returns the drift length variance next to the reconstructed position. Dummy implemented as the reference drift length
      FloatType getRecoDriftLengthVariance() const
      { return getRLWireHit().getRefDriftLengthVariance(); }



      /// Adjust the travel distance by the given value.
      void shiftPerpS(FloatType perpSOffSet)
      { m_perpS += perpSOffSet; }

      /// Getter for the travel distance in the xy projection
      FloatType getPerpS() const
      { return m_perpS; }

      /// Setter for the travel distance in the xy projection
      void setPerpS(const FloatType& perpS)
      { m_perpS = perpS; }

      /// Getter the travel distance on the given drift circle
      /** Calculates the travel distance on the circle by taking the closest approach to the \n
       *  reconstructed hit position as seen from the xy projection and taking the arc length \n
       *  from the reference on the circle. */
      FloatType getPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getRecoPos2D()); }

      /** indicator if the hit is in the cdc or already outside its boundaries.
          Checks for z to be in the range of the wire. */
      bool isInCDC() const;

      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference there is no way to tell \n
       *  if one should use the dot '.' or operator '->' for method look up. \n
       *  So this function defines the -> operator for the object. \n
       *  No matter you have a pointer or an object access is given with '->'*/
      const CDCRecoHit3D* operator->() const { return this; }

      /// Getter for the center of mass. Center of mass is just the reconstructed position
      Vector2D getCenterOfMass2D() const { return getRecoPos2D(); }

      /// Same as getRecoPos2D()
      Vector2D getFrontRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return getRecoPos2D(trajectory2D); }

      /// Same as getRecoPos2D()
      Vector2D getBackRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return getRecoPos2D(trajectory2D); }

      /// Same as getPerpS().
      FloatType getFrontPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Same as getPerpS().
      FloatType getBackPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Calculates the squared distance of the reconstructed position to a circle as see from the transvers plane.
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const
      { FloatType distance = trajectory2D.getDist2D(getRecoPos2D()); return distance * distance; }

      /// Calculates the squared distance in z direction.
      /** Calculates the z position on the fitted sz line with the stored travel distance.\n
       *  Returns the squared difference to the reconstructed position of this reconstructed hit.*/
      FloatType getSquaredZDist(const CDCTrajectorySZ& szTrajectory) const {
        FloatType zDistance = szTrajectory.getZDist(getPerpS(), getRecoPos3D().z());
        return zDistance * zDistance;
      }


    private:
      const CDCRLWireHit* m_rlWireHit;  ///< Memory for the oriented wire hit reference

      Vector3D m_recoPos3D; ///< Memory for the reconstructed hit position
      FloatType m_perpS; ///< Memory for the travel distance as see in the xy projection.

      /// ROOT Macro to make CDCRecoHit3D a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoHit3D, 1);

    }; //class

  } //end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOHIT3D_H
