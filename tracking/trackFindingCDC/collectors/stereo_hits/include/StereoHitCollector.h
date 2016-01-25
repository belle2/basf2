/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/collectors/base/Collector.h>
#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/hough/z0_tanLambda/HitZ0TanLambdaLegendre.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRecoHit3D;
    class CDCRLTaggedWireHit;
    class CDCTrajectory2D;

    class StereoHitCollector : public Collector<CDCTrack, CDCRLTaggedWireHit> {
    public:
      virtual void exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix = "") override;

    public:
      /** Prepare the hough algorithm: initialize the quad trees. */
      virtual void initialize() override
      {
        m_hitQuadTree.setMaxLevel(m_param_quadTreeLevel);
        m_hitQuadTree.initialize();
      }

      /** Raze the quad tree. */
      virtual void terminate() override
      {
        m_hitQuadTree.raze();
      }

    protected:
      /**
       * Create a QuadTree and fill with each unused stereo hit (to be exact: twice for each stereo hit - right and left).
       * The QuadTree has two dimensions: inverse slope in z-direction and z0.
       * Each bin with a high number of items (= stereo hits) in it is stored. Later, the one node with the highest number of items in it is taken
       * and each hit is assigned to the track.
       */
      virtual std::vector<std::pair<const CDCRLTaggedWireHit*, Weight>> match(const CDCTrack& collectorItem,
          const std::vector<CDCRLTaggedWireHit>& collectionList) override;

      virtual void add(CDCTrack& collectorItem, const std::vector<const CDCRLTaggedWireHit*>& matchedItems) override;

    private:
      /// Parameters
      /// Maximum level of the quad tree search.
      unsigned int m_param_quadTreeLevel = 6;
      /// Set to false to skip the B2B check (good for curlers)
      bool m_param_checkForB2BTracks = true;
      /// Minimal number of hits a quad tree node must have to be called a found bin
      unsigned int m_param_minimumNumberOfHits = 5;
      /// Instead of deleting used hits, leave them where they are and check for double assignment later.
      bool m_param_checkForDoubleHits = false;

      /// quad tree implementations
      /// Handler for the hit quad tree
      HitZ0TanLambdaLegendre m_hitQuadTree;

      /**
       * Use the given trajectory to reconstruct the 2d hits in the vector in z direction
       * to match the trajectory perfectly. Then add the newly created reconstructed 3D hit to the given list.
       *
       * WARNING: We *create* CDCRecoHit3Ds here as pointers, but the ownership is handled over to the list.
       * Please delete the hits by yourself.
       */
      void reconstructHit(const CDCRLTaggedWireHit& rlWireHit, std::vector<CDCRecoHit3D>& hitsVector,
                          const CDCTrajectory2D& trackTrajectory, const bool isCurler, const double radius) const;

      void fillHitsVector(std::vector<CDCRecoHit3D>& hitsVector, const CDCTrack& track,
                          const std::vector<CDCRLTaggedWireHit>& wireHits) const;
    };
  }
}
