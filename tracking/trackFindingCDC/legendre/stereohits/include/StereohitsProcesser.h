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

#include <tracking/trackFindingCDC/hough/z0_tanLambda/HitZ0TanLambdaLegendre.h>
#include <tracking/trackFindingCDC/hough/z0_tanLambda/SegmentZ0TanLambdaLegendre.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRLWireHit;
    class CDCTrajectory2D;
    class CDCRecoHit3D;
    class CDCRecoSegment2D;

    /** Class handling the stereo hit adding */
    class StereohitsProcesser {
    public:
      /** Create a quad tree */
      explicit StereohitsProcesser(unsigned int level, bool debugOutput = false, bool checkForB2BTracks = true) :
        m_param_debugOutput(debugOutput), m_level(level), m_checkForB2BTracks(checkForB2BTracks),
        m_newQuadTree(level),
        m_segmentQuadTree(level)
      {
        // Prepare the hough algorithm
        m_newQuadTree.initialize();
        m_segmentQuadTree.initialize();
      }

      /** Destructor razing the quad tree */
      ~StereohitsProcesser()
      {
        m_newQuadTree.raze();
        m_segmentQuadTree.raze();
      }

      /**
       * Create a QuadTree and fill with each unused stereo hit (to be exact: twice for each stereo hit - right and left).
       * The QuadTree has two dimensions: inverse slope in z-direction and z0.
       * Each bin with a high number of items (= stereo hits) in it is stored. Later, the one node with the highest number of items in it is taken
       * and each hit is assigned to the track.
       * */
      void makeHistogramming(CDCTrack& track, unsigned int minimumNumberOfHits);

      /**
       * Create a QuadTree and fill with each unused stereo hit (to be exact: twice for each stereo hit - right and left).
       * The QuadTree has two dimensions: inverse slope in z-direction and z0.
       * Each bin with a high number of items (= stereo hits) in it is stored. Later, the one node with the highest number of items in it is taken
       * and each hit is assigned to the track.
       * */
      void makeHistogrammingWithNewQuadTree(CDCTrack& track, unsigned int minimumNumberOfHits);

      void makeHistogrammingWithSegments(CDCTrack& track, const std::vector<CDCRecoSegment2D>& segments,
                                         unsigned int minimumNumberOfHits);

    private:
      /// We will use CDCRecoHits3D in the QuadTrees.
      typedef const CDCRecoHit3D HitType;

      /**
       * Fill the given vector with all not used rl wire hits that could match the given track as CDCRecoHits3D.
       */
      void fillHitsVector(std::vector<HitType*>& hitsVector, const CDCTrack& track) const;

      /**
       * Add only those hits to the track that are in the node with the maximum number of hits.
       */
      void addMaximumNodeToTrackAndDeleteHits(CDCTrack& track, std::vector<HitType*>& foundStereoHits,
                                              const std::vector<HitType*>& doubledRecoHits, const std::vector<HitType*>& hitsVector) const;

      void reconstructSegment(const CDCRecoSegment2D& segment, std::vector<const CDCRecoSegment3D*>& recoSegments,
                              const CDCTrajectory2D& trackTrajectory, const double maximumPerpS) const;

      bool m_param_debugOutput; /// Flag to turn on debug output
      unsigned int m_level; /// Maximum level of the quad tree search.
      bool m_checkForB2BTracks; /// Set to false to skip the B2B check (good for curlers)
      HitZ0TanLambdaLegendre<const HitType*> m_newQuadTree; /// Handler for the new quad tree (the old one is getting created every event)
      SegmentZ0TanLambdaLegendre<2, 2> m_segmentQuadTree; /// Handler for the new quad tree (the old one is getting created every event)


    };
  }
}
