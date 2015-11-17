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
    class StereoHitsProcesser {
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

      /// Postprocessing: sort by arc length, set all arc lengths to be positive and fit the tracks.
      void postprocessTrack(CDCTrack& track) const;

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
      /// Fill the given vector with all not used rl wire hits that could match the given track as CDCRecoHits3D.
      void fillHitsVector(std::vector<CDCRecoHit3D>& hitsVector, const CDCTrack& track) const;

      /// Fill the given vector with all not used segments that could match the given track as CDCRecoSegment3D.
      void fillSegmentsVector(std::vector<const CDCRecoSegment3D*> recoSegmentsVector, const std::vector<CDCRecoSegment2D>& segments,
                              CDCTrack& track) const;

      /**
       * Use the given trajectory to reconstruct the 2d hits in the segment in z direction
       * to match the trajectory perfectly. Then add the newly created reconstructed 3D segment to the given list.
       *
       * WARNING: We *create* CDCRecoSegment3Ds here as pointers, but the ownership is handles over to the list.
       * Please delete the segments by yourself.
       */
      void reconstructSegment(const CDCRecoSegment2D& segment, std::vector<const CDCRecoSegment3D*>& recoSegments,
                              const CDCTrajectory2D& trackTrajectory) const;

      /**
       * Use the given trajectory to reconstruct the 2d hits in the vector in z direction
       * to match the trajectory perfectly. Then add the newly created reconstructed 3D hit to the given list.
       *
       * WARNING: We *create* CDCRecoHit3Ds here as pointers, but the ownership is handles over to the list.
       * Please delete the hits by yourself.
       */
      void reconstructHit(const CDCRLWireHit& rlWireHit, std::vector<CDCRecoHit3D>& hitsVector,
                          const CDCTrajectory2D& trackTrajectory, const bool isCurler, const double radius) const;

      /// Parameters
      /// Maximum level of the quad tree search.
      unsigned int m_param_quadTreeLevel = 6;
      /// Set to false to skip the B2B check (good for curlers)
      bool m_param_checkForB2BTracks = true;
      /// Minimal number of hits a quad tree node must have to be called a found bin
      unsigned int m_param_minimumNumberOfHits = 5;

      /// quad tree implementations
      /// Handler for the hit quad tree
      HitZ0TanLambdaLegendre m_hitQuadTree;
      /// Handler for the segment quad tree
      SegmentZ0TanLambdaLegendre m_segmentQuadTree;
    };
  }
}
