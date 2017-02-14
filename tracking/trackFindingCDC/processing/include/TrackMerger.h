/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun, Oliver Frost                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCTrack;
    class CDCWireHit;
    class CDCTrajectory2D;

    /// Utility structure implementing functionality for the axial track merging used after the legendre tree search.
    class TrackMerger {
    public:
      /// Static class only.
      TrackMerger() = delete;

      /**
       *  The track finding often finds two curling tracks, originating from the same particle.
       *  This function merges them.
       */
      static void doTracksMerging(std::vector<CDCTrack>& axialTracks,
                                  const std::vector<const CDCWireHit*>& allAxialWireHits,
                                  double minimum_probability_to_be_merged = 0.85);

    private:
      /**
       *  Searches for the best candidate to merge this track to.
       *  @param track   track for which we try to find merging partner
       *  @param tracks  search range of tracks
       *  @return        a pointer to the best fit candidate including a fit probability
       *  @retval        <tt>{nullptr, 0}<\tt> in case no match was found
       */
      template <class ACDCTracks>
      static WithWeight<MayBePtr<CDCTrack> > calculateBestTrackToMerge(CDCTrack& track, ACDCTracks& tracks);

      /**
       *  Fits the hit content of both tracks in a common fit repeated with an annealing schedule removing far away hits
       *
       *  @return Some measure of fit probability
       */
      static double doTracksFitTogether(CDCTrack& track1, CDCTrack& track2);

      /**
       *  Remove all hits that are further than factor * driftlength waay from the trajectory
       *
       *  @param factor gives a number how far the hit is allowed to be.
       */
      static void removeStrangeHits(double factor, std::vector<const CDCWireHit*>& wireHits, CDCTrajectory2D& trajectory);

      /**
       *  Function to merge two track candidates.
       *  The hits of track2 are deleted and transfered to track1 and the track1 is resorted.
       *  The method also applys some postprocessing and splits the track1 in case it appears
       *  to contain two back-to-back arms,
       */
      static void mergeTracks(CDCTrack& track1,
                              CDCTrack& track2,
                              const std::vector<const CDCWireHit*>& allAxialWireHits);
    };
  }
}
