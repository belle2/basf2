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

    /**
     * Class handling the stereo hit adding.
     * Its job is to create and fill the two possible quad trees (for hits or segments) and raze them in the end.
     *
     * Both quad trees are initialized and razed in the moment, although only the one for hits is used in the standard reconstruction.
     * This may change in the future.
     */
    class StereohitsProcesser {
    public:
      /** Prepare the hough algorithm: initialize the quad trees. */
      void initialize()
      {

        m_hitQuadTree.setMaxLevel(m_param_quadTreeLevel);
        m_hitQuadTree.initialize();

        m_segmentQuadTree.setMaxLevel(m_param_quadTreeLevel);
        m_segmentQuadTree.initialize();
      }

      /** Raze the quad tree. */
      void terminate()
      {
        m_hitQuadTree.raze();
        m_segmentQuadTree.raze();
      }

      /**
       * Create a QuadTree and fill with each unused stereo hit (to be exact: twice for each stereo hit - right and left).
       * The QuadTree has two dimensions: inverse slope in z-direction and z0.
       * Each bin with a high number of items (= stereo hits) in it is stored. Later, the one node with the highest number of items in it is taken
       * and each hit is assigned to the track.
       */
      void addStereoHitsWithQuadTree(CDCTrack& track);

      /**
       * Create a QuadTree and fill with each unused segment (to be exact: twice for each segment - one normal, one reversed to get the orientation correct).
       * The QuadTree has two dimensions: inverse slope in z-direction and z0.
       * Each bin with a high number of items (= stereo hits) in it is stored. Later, the one node with the highest number of items in it is taken
       * and each hit is assigned to the track.
       */
      void addStereoHitsWithQuadTree(CDCTrack& track, const std::vector<CDCRecoSegment2D>& segments);

      /** Postprocessing: sort by arc length, set all arc lengths to be positive and fit the tracks. */
      void postprocessTrack(CDCTrack& track);

      /// Return the bare value of the parameter check for B2B tracks.
      bool getCheckForB2BTracksValue() const
      {
        return m_param_checkForB2BTracks;
      }

      /// Return the bare value of the parameter minimum number of hits.
      unsigned int getMinimumNumberOfHitsValue() const
      {
        return m_param_minimumNumberOfHits;
      }

      /// Return the bare value of the parameter quad tree level.
      unsigned int getQuadTreeLevelValue() const
      {
        return m_param_quadTreeLevel;
      }

      /// Return the reference to the parameter check for B2B tracks.
      bool& getCheckForB2BTracksParameter()
      {
        return m_param_checkForB2BTracks;
      }

      /// Return the reference to the parameter minimum number of hits.
      unsigned int& getMinimumNumberOfHitsParameter()
      {
        return m_param_minimumNumberOfHits;
      }

      /// Return the reference to the parameter quad tree level.
      unsigned int& getQuadTreeLevelParameter()
      {
        return m_param_quadTreeLevel;
      }

    private:

      /// Utilities
      /**
       * Fill the given vector with all not used rl wire hits that could match the given track as CDCRecoHits3D.
       */
      void fillHitsVector(std::vector<const CDCRecoHit3D*>& hitsVector, const CDCTrack& track) const;

      /**
       * Add only those hits to the track that are in the node with the maximum number of hits.
       */
      void addMaximumNodeToTrackAndDeleteHits(CDCTrack& track, std::vector<const CDCRecoHit3D*>& foundStereoHits,
                                              const std::vector<const CDCRecoHit3D*>& doubledRecoHits, const std::vector<const CDCRecoHit3D*>& hitsVector) const;

      /**
       * Use the given trajectory to reconstruct the 2d hits in the segment in z direction
       * to match the trajectory perfectly. Then add the newly created reconstructed 3D segment to the given list.
       */
      void reconstructSegment(const CDCRecoSegment2D& segment, std::vector<const CDCRecoSegment3D*>& recoSegments,
                              const CDCTrajectory2D& trackTrajectory, const double maximumPerpS) const;

      /// Parameters
      unsigned int m_param_quadTreeLevel = 6;        /// Maximum level of the quad tree search.
      bool m_param_checkForB2BTracks = true;         /// Set to false to skip the B2B check (good for curlers)
      unsigned int m_param_minimumNumberOfHits = 5;  /// Minimal number of hits a quad tree node must have to be called a found bin

      /// quad tree implementations
      HitZ0TanLambdaLegendre m_hitQuadTree;          /// Handler for the hit quad tree
      SegmentZ0TanLambdaLegendre m_segmentQuadTree;  /// Handler for the segment quad tree
    };
  }
}
