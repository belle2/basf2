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

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCStereoRecoSegment2D.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class implementing the z coordinate over travel distance line fit.
    class CDCSZFitter {
    public:
      /// Getter for a standard sz line fitter instance.
      static const CDCSZFitter& getFitter();

      /// Returns a fitted trajectory
      CDCTrajectorySZ fit(const CDCStereoRecoSegment2D& stereoSegment,
                          const CDCTrajectory2D& axialTrajectory2D) const
      {
        CDCTrajectorySZ trajectorySZ;
        update(trajectorySZ, stereoSegment, axialTrajectory2D);
        return trajectorySZ;
      }

      /// Fits a linear sz trajectory to the z and s coordinates in the stereo segment.
      CDCTrajectorySZ fit(const CDCRecoSegment3D& segment3D) const
      {
        CDCTrajectorySZ trajectorySZ;
        update(trajectorySZ, segment3D);
        return trajectorySZ;
      }

      /// Updates the trajectory of the axial stereo segment pair inplace
      void update(const CDCSegmentPair& segmentPair) const;

      /// Update the given sz trajectory reconstructing the stereo segment with a near by axial segment
      void update(CDCTrajectorySZ& trajectorySZ,
                  const CDCStereoRecoSegment2D& stereoSegment,
                  const CDCTrajectory2D& axialTrajectory2D) const;

      /**
       * Update the trajectory with a fit in the sz direction to the three dimensional hits.
       * Do only update the trajectory if the number of usable stereo hits exceeds 3.
       */
      void update(CDCTrajectorySZ& trajectory,
                  const std::vector<CDCRecoHit3D>& recoHits3D) const
      {
        CDCObservations2D observationsSZ;
        appendSZ(observationsSZ, recoHits3D);
        if (observationsSZ.size() > 3) {
          update(trajectory, observationsSZ);
        }
      }


      /// Fits a linear sz trajectory to the s and z coordinates given in the observations.
      CDCTrajectorySZ fit(CDCObservations2D& observationsSZ) const
      {
        CDCTrajectorySZ trajectorySZ;
        update(trajectorySZ, observationsSZ);
        return trajectorySZ;
      }



    private:
      /// Appends the s and z values of all given hits to the observation matrix
      void appendSZ(CDCObservations2D& observationsSZ, const std::vector<CDCRecoHit3D>& recoHits3D) const
      { for (const CDCRecoHit3D& recoHit3D : recoHits3D) appendSZ(observationsSZ, recoHit3D); }

      /** Appends the s and z value of the given hit to the observation matrix
       *
       *  In case the hit do contains a NaN value it will not be appended to the observations
       *
       *  @return   The number of hits appended which is 0 or 1 here.*/
      size_t appendSZ(CDCObservations2D& observationsSZ, const Belle2::TrackFindingCDC::CDCRecoHit3D& recoHit3D) const
      {
        // Translate the drift length uncertainty to a uncertainty in z
        // by the taking the projected wire vector part parallel to the displacement
        // as a proportionality factor to the z direction.
        const CDCWire& wire = recoHit3D.getWire();
        const Vector3D& wireVector = wire.getWireVector();
        const Vector2D disp2D = recoHit3D.getRecoDisp2D();
        const double driftlengthVariance = recoHit3D.getRecoDriftLengthVariance();

        double dispNorm = disp2D.norm();

        double zeta = 1.0;
        if (dispNorm == 0.0) {
          zeta = wireVector.xy().norm() / wireVector.z();
        } else {
          zeta = wireVector.xy().dot(disp2D) / wireVector.z() / dispNorm;
        }

        //double weight = 1.0;
        double weight = zeta * zeta / driftlengthVariance;

        size_t appended_hit = appendSZ(observationsSZ, recoHit3D.getArcLength2D(), recoHit3D.getRecoPos3D().z(), weight);
        // if (not appended_hit){
        //   B2WARNING("CDCRecoHit3D was not appended as SZ observation.");
        //   B2WARNING("S: " << recoHit3D.getPerpS());
        //   B2WARNING("Z: " << recoHit3D.getRecoPos3D().z());
        //   B2WARNING("Reconstructed position: " << recoHit3D.getRecoPos2D());
        //   B2WARNING("Reconstructed displacement: " << recoHit3D.getRecoDisp2D());
        //   B2WARNING("Reconstructed displacement norm: " << dispNorm);
        //   B2WARNING("Reconstructed displacement norm eq 0: " << (dispNorm == 0.0));
        //   B2WARNING("Stereo type: " << recoHit3D.getStereoType());
        //   B2WARNING("Wire vector: " << wireVector);
        //   B2WARNING("Zeta: " << zeta);
        //   B2WARNING("Variance: " << 1.0 / weight);
        //   B2WARNING("Weight: " << weight << std::endl);
        // }
        return appended_hit;
      }

      /// Appends the s and z value of the given hit to the observation matrix
      size_t appendSZ(CDCObservations2D& observationsSZ, const double s, const double z, const double weight = 1.0) const
      { return observationsSZ.append(s, z, 0.0, weight); }

    public:
      /// Update the trajectory with a fit to the observations.
      void update(CDCTrajectorySZ& trajectorySZ, CDCObservations2D& observationsSZ) const;

      /// Returns the fitted sz trajectory of the track with the z-information of all stereo hits of the number
      /// of stereo hits is big enough. Else return the basic assumption.
      CDCTrajectorySZ fitWithStereoHits(CDCTrack& track) const
      {
        CDCObservations2D observationsSZ;
        for (const CDCRecoHit3D& recoHit : track) {
          if (recoHit.getStereoType() != EStereoType::c_Axial) {
            appendSZ(observationsSZ, recoHit);
          }
        }

        if (observationsSZ.size() > 3) {
          CDCTrajectorySZ szTrajectory;
          update(szTrajectory, track);
          return szTrajectory;
        } else {
          return CDCTrajectorySZ::basicAssumption();
        }
      }

    }; //class
  } // end namespace TrackFindingCDC
} // namespace Belle2
