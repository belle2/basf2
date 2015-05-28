/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <utility>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    class TrackHit;
    class TrackCandidate;
    class StereoHit;
    class CDCTrack;
    class CDCRLWireHit;

    class StereohitsProcesser {
    public:

      /**
       * Create a QuadTree and fill with each unused stereo hit (to be exact: twice for each stereo hit - right and left).
       * The QuadTree has two dimensions: slope in z-direction and z0.
       * Each bin with a high number of items (= stereo hits) in it is stored. Later, the one node with the highest number of items in it is taken
       * and each hit is assigned to the track.
       * */
      void makeHistogramming(CDCTrack& track);

      /** Return displacements of the stereohit against the track */
      // has to be refactored?
      std::pair<StereoHit, StereoHit> getDisplacements(TrackCandidate* cand, TrackHit* hit, int trackCharge = 0);

      /** Assign stereohits to the track with known polar angle */
      // has to be refactored? unused?
      void assignStereohitsByAngle(TrackCandidate* cand, double theta, std::vector<TrackHit*>& stereohits, double Z0 = 0.);

    private:

      /** Return displacement (inner or outer) of the stereohit against the track */
      // has to be refactored
      StereoHit getDisplacement(TrackCandidate* cand, TrackHit* hit, int InnerOuter);

      /** Return position of the hit on the track expressed in rads */
      // has to be refactored
      double getAlpha(TrackCandidate* cand, std::pair<double, double> pos);

      /** Returns a bool if the rlWire can be matched to a track. This is calculated with the track charge and the superlayer information */
      bool rlWireHitMatchesTrack(const CDCRLWireHit& rlWireHit, const CDCTrack& track);
    };
  }


}
