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
#include <tracking/trackFindingCDC/collectors/base/FirstMatchCollector.h>
#include <tracking/trackFindingCDC/collectors/quadtree/QuadTreeBasedMatcher.h>
#include <framework/core/ModuleParamList.h>
#include <tracking/trackFindingCDC/hough/z0_tanLambda/HitZ0TanLambdaLegendre.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRecoHit3D;
    class CDCRLTaggedWireHit;
    class CDCTrajectory2D;

    class StereoHitTrackMatcher : public QuadTreeBasedMatcher<HitZ0TanLambdaLegendre> {
    public:
      typedef CDCTrack CollectorItem;
      typedef CDCRLTaggedWireHit CollectionItem;

      virtual void exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix = "") override;

      /**
       * Create a QuadTree and fill with each unused stereo hit (to be exact: twice for each stereo hit - right and left).
       * The QuadTree has two dimensions: inverse slope in z-direction and z0.
       * Each bin with a high number of items (= stereo hits) in it is stored. Later, the one node with the highest number of items in it is taken
       * and each hit is assigned to the track.
       */
      std::vector<WithWeight<const CollectionItem*>> match(const CollectorItem& collectorItem,
                                                           const std::vector<CollectionItem>& collectionList);

    private:
      /// Parameters
      /// Set to false to skip the B2B check (good for curlers)
      bool m_param_checkForB2BTracks = true;

      typedef std::pair<CDCRecoHit3D, const CDCRLTaggedWireHit*> CDCRecoHitWithRLPointer;

      /**
       * Use the given trajectory to reconstruct the 2d hits in the vector in z direction
       * to match the trajectory perfectly. Then add the newly created reconstructed 3D hit to the given list.
       *
       * WARNING: We *create* CDCRecoHit3Ds here as pointers, but the ownership is handled over to the list.
       * Please delete the hits by yourself.
       */
      void reconstructHit(const CDCRLTaggedWireHit& rlWireHit, std::vector<CDCRecoHitWithRLPointer>& hitsVector,
                          const CDCTrajectory2D& trackTrajectory, const bool isCurler, const double radius) const;
    };
  }
}
