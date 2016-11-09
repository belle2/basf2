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

#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <framework/core/ModuleParamList.h>

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
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix) override;

      /// Initialize the filter and the quad tree.
      virtual void initialize() override;

      /// Terminate the filter and the quad tree.
      virtual void terminate() override;

      /**
       * Create a QuadTree and fill with each unused stereo hit (to be exact: twice for each stereo hit - right and left).
       * The QuadTree has two dimensions: inverse slope in z-direction and z0.
       * Each bin with a high number of items (= stereo hits) in it is stored. Later, the one node with the highest number of items in it is taken
       * and each hit is assigned to the track.
       */
      std::vector<WithWeight<const CDCRLWireHit*> >
      match(const CDCTrack& track, const std::vector<CDCRLWireHit>& rlWireHits);

    private:
      /// Parameters
      /// Set to false to skip the B2B check (good for curlers).
      bool m_param_checkForB2BTracks = true;

      /// Set to false to skip the in-wire-bound check (good for second stage).
      double m_param_checkForInWireBoundsFactor = 1.0;

      /// The used filter.
      ChooseableFilter<StereoHitFilterFactory> m_stereoHitFilter;
    };
  }
}
