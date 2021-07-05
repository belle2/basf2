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

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/topology/EStereoKind.h>
#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/numerics/ERotation.h>

namespace Belle2 {
  class CDCSimHit;
  class CDCHit;

  namespace TrackFindingCDC {
    // Forward declaration.
    class CDCTrajectory3D;
    class CDCTrajectory2D;
    class CDCTrajectorySZ;
    class CDCRecoHit2D;
    class CDCWireHit;
    class CDCWire;

    /**
     *  Class representing a three dimensional reconstructed hit.
     *  A recohit represents a likely point where the particle went through. It is always assoziated with a
     *  wire hit it seeks to reconstruct. The reconstructed point is stored as an absolut position from the
     *  origin / interaction point. In addition the reconstructed hit takes a right left passage information
     *  which indicates if the hit wire lies to the right or to the left of the particle trajectory causing the hit.
     *  The later readily indicates a flight direction from the reconstructed hit, if a tangential approch of
     *  the trajectory to the drift circle is assumed.
     *
     *  Finally we want to estimate the travel distance to z coordinate relation of the particle trajectory.
     *  Therefore the three dimensional reconstructed hit stores the travel distance as seen from the xy projection
     *  ( arcLength2D ) it took to get to hit. This variable can be calculated from the trajectory circle fitted in
     *  the two dimensional tracking as the arc length.
     */
    class CDCRecoHit3D  {
    public:
      /// Default constructor for ROOT
      CDCRecoHit3D() = default;

      /// Constructor taking all stored variables of the reconstructed hit.
      CDCRecoHit3D(const CDCRLWireHit& rlWireHit, const Vector3D& recoPos3D, double arcLength2D = 0);

      /**
       *  Constructs a three dimensional reconstructed hit from a sim hit and the assoziated wirehit.
       *  This translates the sim hit to a reconstructed hit mainly to be able to compare the
       *  reconstructed values from the algorithm with the Monte Carlo information.
       *  It merely takes the position from the sim hit and calculates the right left passage information.
       *  Since only the time is present in the sim hit but not the travel distance this parameter is just set
       *  NAN!
       */
      static CDCRecoHit3D fromSimHit(const CDCWireHit* wireHit, const CDCSimHit& simHit);

      /**
       *  Reconstructs the three dimensional hit from the two dimensional and the two dimensional trajectory.
       *  For two dimensional reconstructed hits on axial wires this reconstructs
       *  the xy position and the transvers travel distance. The z coordinate cannot
       *  be determined since the projection can not show any information about it. It
       *  is always set to NAN for axial hits.
       *
       *  For two dimensional reconstructed hits on stereo wires however the deviation of the xy position
       *  can be used to get z information. The reconstucted hit lies exactly on the fitted trajectory
       *  as seen from the xy plane. Hence also xy position and transvers travel distance are available.
       *  Only the stereo hits have then the full information to go head and make the sz trajectory.
       */
      static CDCRecoHit3D reconstruct(const CDCRecoHit2D& recoHit2D,
                                      const CDCTrajectory2D& trajectory2D);

      /**
       *  Reconstructs the three dimensional hit from the wire hit and the given right left passage
       *  information by shifting it to a z coordinate, where it touches the two dimensional trajectory
       *  from the side indicated by the right left passage.
       */
      static CDCRecoHit3D reconstruct(const CDCRLWireHit& rlWireHit,
                                      const CDCTrajectory2D& trajectory2D);

      /**
       *  Reconstructs the three dimensional hit from the wire hit and the given right left passage
       *  information by shifting it to a z coordinate, where it touches the two dimensional trajectory
       *  from the side indicated by the right left passage.
       */
      static CDCRecoHit3D reconstruct(const CDCWireHit* wireHit,
                                      ERightLeft rlInfo,
                                      const CDCTrajectory2D& trajectory2D);

      /**
       *  Reconstruct a three dimensional hit from a wire hit (as in reconstruct(rlWireHit, trajectory2D)),
       *  but this time use a wire hit without a right-left information as an input.
       *  The right-left information is chosen to be consistent with the
       *  reference position of the wire and the trajectory passed in to that function (by checking the sign of
       *  trackTrajectory2D.getDist2D(wireHit.getRefPos2D())).
       *
       *  This function is only sensible for axial hits (and asserts this).
       */
      static CDCRecoHit3D reconstructNearest(const CDCWireHit* axialWireHit,
                                             const CDCTrajectory2D& trajectory2D);

      /// Convenience function to call the other reconstruct method with the sz- and 2d-trajectory contained in the 3d one.
      static CDCRecoHit3D reconstruct(const CDCRecoHit2D& recoHit,
                                      const CDCTrajectory3D& trajectory3D);

      /**
       *  Deprecated - try to use the method above for the same purpose.
       *
       *  Reconstructs the three dimensional hit from the two dimensional,
       *  the two dimensional trajectory and sz trajectory.
       *  For two dimensional reconstructed hits on axial wires this reconstructs
       *
       *  the xy position and the transvers travel distance. The z coordinate is then determined
       *  by evaluating the sz trajectory at the just calculated travel distance. Note that it is important
       *  that both circle trajectory and sz trajectory share a common reference point.
       *
       *  For two dimensional reconstructed hits on stereo wires the transerse travel distance is obtained
       *  as in the reconstuct() method before. However the z coordinate is set to the value of the sz trajectory
       *  at just calculated the transvers travel distance, since the trajectory should be more exact than the shifting
       *  along the wire.
       */
      static CDCRecoHit3D reconstruct(const CDCRecoHit2D& recoHit2D,
                                      const CDCTrajectory2D& trajectory2D,
                                      const CDCTrajectorySZ& trajectorySZ);

    public:
      /**
       *  Constructs the average of two reconstructed hit positions.
       *  Averages the hit positions and the travel distance. The function averages only reconstructed hits
       *  assoziated with the same wire hit. If not all recostructed hits are on the same wire hit, the first hit
       *  is returned unchanged. Also averages the right left passage information with averageInfo().
       */
      static CDCRecoHit3D average(const CDCRecoHit3D& first,
                                  const CDCRecoHit3D& second);

      /**
       *  Turns the orientation in place.
       *  Changes the sign of the right left passage information,
       *  since the position remains the same by this reversion.
       */
      void reverse();

      /// Returns the recohit with the opposite right left information.
      CDCRecoHit3D reversed() const;

      /// Make the wire hit automatically castable to its underlying cdcHit.
      operator const Belle2::CDCHit* () const
      {
        return static_cast<const CDCHit*>(getRLWireHit());
      }

      /// Equality comparision based on wire hit, right left passage information and reconstructed position.
      bool operator==(const CDCRecoHit3D& other) const
      {
        return getRLWireHit() == other.getRLWireHit() and
               getRLInfo() == other.getRLInfo() and
               getRecoPos3D() == other.getRecoPos3D();
      }

      /**
       *  Total ordering relation based on wire hit, right left passage
       *  information and position information in this order of importance.
       */
      bool operator<(const CDCRecoHit3D& other) const
      {
        return (getRLWireHit() < other.getRLWireHit() or
                (getRLWireHit() == other.getRLWireHit() and
                 getRecoPos3D() < other.getRecoPos3D()));
      }

      /// Defines wires and the three dimensional reconstructed hits as coaligned.
      friend bool operator<(const CDCRecoHit3D& recoHit3D, const CDCWire& wire)
      {
        return recoHit3D.getRLWireHit() < wire;
      }

      /// Defines wires and the three dimensional reconstructed hits as coaligned.
      friend bool operator<(const CDCWire& wire, const CDCRecoHit3D& recoHit3D)
      {
        return wire < recoHit3D.getRLWireHit();
      }

      /// Defines wire hits and the three dimensional reconstructed hits as coaligned.
      friend bool operator<(const CDCRecoHit3D& recoHit3D, const CDCWireHit& wireHit)
      {
        return recoHit3D.getRLWireHit() < wireHit;
      }

      /// Defines wire hits and the three dimensional reconstructed hits as coaligned.
      friend bool operator<(const CDCWireHit& wireHit, const CDCRecoHit3D& recoHit3D)
      {
        return wireHit < recoHit3D.getRLWireHit();
      }

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

      /// Getter for the wire.
      const CDCWire& getWire() const
      {
        return getRLWireHit().getWire();
      }

      /// Checks if the reconstructed hit is assoziated with the give wire.
      bool isOnWire(const CDCWire& wire) const
      {
        return getRLWireHit().isOnWire(wire);
      }

      /// Getter for the wire hit.
      const CDCWireHit& getWireHit() const
      {
        return getRLWireHit().getWireHit();
      }

      /// Checks if the reconstructed hit is assoziated with the give wire hit.
      bool hasWireHit(const CDCWireHit& wireHit) const
      {
        return getRLWireHit().hasWireHit(wireHit);
      }

      /// Getter for the oriented wire hit.
      const CDCRLWireHit& getRLWireHit() const
      {
        return m_rlWireHit;
      }

      /// Setter for the oriented wire hit assoziated with the reconstructed hit.
      void setRLWireHit(const CDCRLWireHit& rlWireHit)
      {
        m_rlWireHit = rlWireHit;
      }

      /**
       *  Getter for the right left passage information.
       *  Returns the right left passage information as see in the xy projection.
       *  It gives if the wire lies on the right or on the left side of the track \n
       *  as you at the xy projection.
       */
      ERightLeft getRLInfo() const
      {
        return getRLWireHit().getRLInfo();
      }

      /// Setter the right left passage information.
      void setRLInfo(ERightLeft rlInfo)
      {
        m_rlWireHit.setRLInfo(rlInfo);
      }

      /// Getter for the reference position of the wire.
      const Vector2D& getRefPos2D() const
      {
        return getRLWireHit().getRefPos2D();
      }

      /// Getter for the 3d position of the hit.
      const Vector3D& getRecoPos3D() const
      {
        return m_recoPos3D;
      }

      /// Setter for the 3d position of the hit.
      void setRecoPos3D(const Vector3D& recoPos3D)
      {
        m_recoPos3D = recoPos3D;
      }

      /// Getter for the 2d position of the hit.
      const Vector2D& getRecoPos2D() const
      {
        return getRecoPos3D().xy();
      }

      /// Getter for the z coordinate of the reconstructed position.
      double getRecoZ() const
      {
        return getRecoPos3D().z();
      }

      /// Gets the displacement from the wire position in the xy plain at the reconstructed position.
      Vector2D getRecoDisp2D() const;

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

      /**
       *  Constructs a two dimensional reconstructed hit by
       *  carrying out the stereo ! projection to the wire reference postion.
       */
      CDCRecoHit2D getRecoHit2D() const;

      /**
       *  Constructs a two dimensional reconstructed hit by
       *  carrying out the stereo ! projection to the wire reference postion.
       */
      CDCRecoHit2D stereoProjectToRef() const;

      /// Returns the position of the wire in the xy plain the reconstructed position is located in.
      Vector2D getRecoWirePos2D() const;

      /// Scales the displacement vector in place to lie on the dirft circle.
      void snapToDriftCircle(bool switchSide = false);

      /**
       *  Returns the drift length next to the reconstructed position.
       *  Dummy implemented as the reference drift length.
       */
      double getSignedRecoDriftLength() const
      {
        return getRLWireHit().getSignedRefDriftLength();
      }

      /// Setter to update the drift length of the hit
      void setRecoDriftLength(double driftLength, bool snapRecoPos);

      /**
       *  Returns the drift length variance next to the reconstructed position.
       *  Dummy implemented as the reference drift length.
       */
      double getRecoDriftLengthVariance() const
      {
        return getRLWireHit().getRefDriftLengthVariance();
      }

      /// Adjust the travel distance by the given value.
      void shiftArcLength2D(double arcLength2DOffSet)
      {
        m_arcLength2D += arcLength2DOffSet;
      }

      /// Getter for the travel distance in the xy projection.
      double getArcLength2D() const
      {
        return m_arcLength2D;
      }

      /// Setter for the travel distance in the xy projection.
      void setArcLength2D(const double arcLength2D)
      {
        m_arcLength2D = arcLength2D;
      }

      /**
       *  Indicator if the hit is in the cdc (scaled by the factor) or already outside its boundaries.
       *  Checks for z to be in the range of the wire.
       */
      bool isInCellZBounds(const double factor = 1) const;

    private:
      /// Memory for the oriented wire hit reference.
      CDCRLWireHit m_rlWireHit;

      /// Memory for the reconstructed hit position.
      Vector3D m_recoPos3D;

      /// Memory for the travel distance as see in the xy projection.
      double m_arcLength2D = 0;
    };
  }
}
