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

#include <tracking/trackFindingCDC/hough/z0_zslope/HitZ0ZSlopeLegendre.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRLWireHit;
    class CDCTrajectory2D;
    class CDCRecoHit3D;

    /** Class handling the stereo hit adding */
    class StereohitsProcesser {
    public:
      /** Create a quad tree */
      explicit StereohitsProcesser(unsigned int level, bool debugOutput = false) :
        m_param_debugOutput(debugOutput), m_level(level), m_newQuadTree(level)
      {
        // Prepare the hough algorithm
        m_newQuadTree.initialize();
      }

      /** Destructor razing the quad tree */
      ~StereohitsProcesser()
      {
        m_newQuadTree.raze();
      }

      /**
       * Create a QuadTree and fill with each unused stereo hit (to be exact: twice for each stereo hit - right and left).
       * The QuadTree has two dimensions: inverse slope in z-direction and z0.
       * Each bin with a high number of items (= stereo hits) in it is stored. Later, the one node with the highest number of items in it is taken
       * and each hit is assigned to the track.
       * */
      void makeHistogramming(CDCTrack& track, unsigned int m_param_minimumHits);

      /**
       * Create a QuadTree and fill with each unused stereo hit (to be exact: twice for each stereo hit - right and left).
       * The QuadTree has two dimensions: inverse slope in z-direction and z0.
       * Each bin with a high number of items (= stereo hits) in it is stored. Later, the one node with the highest number of items in it is taken
       * and each hit is assigned to the track.
       * */
      void makeHistogrammingWithNewQuadTree(CDCTrack& track, unsigned int m_param_minimumHits);

    private:
      /// We will use CDCRecoHits3D in the QuadTrees.
      typedef const CDCRecoHit3D HitType;

      /** Returns a bool if the rlWire can be matched to a track. This is calculated with the track charge and the superlayer information */
      bool rlWireHitMatchesTrack(const CDCRLWireHit& rlWireHit, const CDCTrajectory2D& trajectory2D) const;

      /**
       * Fill the given vector with all not used rl wire hits that could match the given track as CDCRecoHits3D.
       */
      void fillHitsVector(std::vector<HitType*>& hitsVector, const CDCTrack& track) const;

      /**
       * Add only those hits to the track that are in the node with the maximum number of hits.
       */
      void addMaximumNodeToTrackAndDeleteHits(CDCTrack& track, std::vector<HitType*>& foundStereoHits,
                                              const std::vector<HitType*>& doubledRecoHits, const std::vector<HitType*>& hitsVector) const;

      bool m_param_debugOutput; /// Flag to turn on debug output
      unsigned int m_level; /// Maximum level of the quad tree search.
      HitZ0ZSlopeLegendre<const HitType*> m_newQuadTree; /// Handler for the new quad tree (the old one is getting created every event)


    };
  }
}
