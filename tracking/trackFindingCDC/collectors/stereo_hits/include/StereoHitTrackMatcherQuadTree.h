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
#include <tracking/trackFindingCDC/collectors/base/QuadTreeBasedMatcher.h>

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
    class CDCRLWireHit;
    class CDCTrajectory2D;

    /**
     * A matcher algorithm for using a stereo quad tree for matching rl tagged wire hits
     * to tracks. After the quad tree has given a yes to a match, the configured filter is used to
     * give a weight to the relation.
     */
    template <class HoughTree>
    class StereoHitTrackMatcherQuadTree : public QuadTreeBasedMatcher<HoughTree> {
      /// Super class
      using Super = QuadTreeBasedMatcher<HoughTree>;
    public:
      /// Use tracks as collector items.
      using CollectorItem = CDCTrack;
      /// Use rl tagged wire hits a collection items.
      using CollectionItem = CDCRLWireHit;

      /// Expose the parameters to the module.
      virtual void exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix = "") override;

      /**
       * Create a QuadTree and fill with each unused stereo hit (to be exact: twice for each stereo hit - right and left).
       * The QuadTree has two dimensions: inverse slope in z-direction and z0.
       * Each bin with a high number of items (= stereo hits) in it is stored. Later, the one node with the highest number of items in it is taken
       * and each hit is assigned to the track.
       */
      std::vector<WithWeight<const CDCRLWireHit*>>
                                                match(const CDCTrack& track, const std::vector<CDCRLWireHit>& rlWireHits);

      /** Initialize the filter and the quad tree. */
      virtual void initialize() override
      {
        Super::initialize();

        m_stereoHitFilter = m_filterFactory.create();
        m_stereoHitFilter->initialize();

        if (m_stereoHitFilter->needsTruthInformation()) {
          StoreArray <CDCSimHit>::required();
          StoreArray <MCParticle>::required();
        }
      }

      /** Terminate the filter and the quad tree. */
      virtual void terminate() override
      {
        Super::terminate();

        m_stereoHitFilter->terminate();
      }

    private:
      /// Parameters
      /// Set to false to skip the B2B check (good for curlers).
      bool m_param_checkForB2BTracks = true;
      /// Set to false to skip the in-wire-bound check (good for second stage).
      double m_param_checkForInWireBoundsFactor = 1.0;

      /// The filter factory we use to create a filter which should later decide which connections to take if there is more than one possibility.
      StereoHitFilterFactory m_filterFactory;
      /// The used filter.
      std::unique_ptr<BaseStereoHitFilter> m_stereoHitFilter;

      /// Helper function to pass the parameters to the filter.
      Weight getWeight(const CDCRecoHit3D& recoHit, const Z0TanLambdaBox& node, const CDCTrack& track) const;

    };
  }
}
