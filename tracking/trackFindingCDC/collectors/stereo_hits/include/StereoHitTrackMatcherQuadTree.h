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

#include <tracking/trackFindingCDC/hough/z0_tanLambda/HitZ0TanLambdaLegendre.h>

#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitFilter.h>
#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitFilterFactory.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/core/ModuleParamList.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRecoHit3D;
    class CDCRLTaggedWireHit;
    class CDCTrajectory2D;

    class StereoHitTrackMatcherQuadTree : public QuadTreeBasedMatcher<HitZ0TanLambdaLegendre> {
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

      virtual void initialize() override
      {
        QuadTreeBasedMatcher<HitZ0TanLambdaLegendre>::initialize();

        m_stereoHitFilter = std::move(m_filterFactory.create());
        m_stereoHitFilter->initialize();

        if (m_stereoHitFilter->needsTruthInformation()) {
          StoreArray <CDCSimHit>::required();
          StoreArray <MCParticle>::required();
        }
      }

      virtual void terminate() override
      {
        QuadTreeBasedMatcher<HitZ0TanLambdaLegendre>::terminate();

        m_stereoHitFilter->terminate();
      }

    private:
      /// Parameters
      /// Set to false to skip the B2B check (good for curlers)
      bool m_param_checkForB2BTracks = true;
      StereoHitFilterFactory m_filterFactory;
      std::unique_ptr<BaseStereoHitFilter> m_stereoHitFilter;

      Weight getWeight(const CDCRecoHit3D& recoHit, const Z0TanLambdaBox& node, const CDCTrack& track) const;

    };
  }
}
