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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/findlets/minimal/RLTaggedWireHitCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackSZFitter.h>

#include <tracking/trackFindingCDC/collectors/matchers/StereoHitTrackQuadTreeMatcher.h>
#include <tracking/trackFindingCDC/collectors/selectors/SingleMatchSelector.h>
#include <tracking/trackFindingCDC/collectors/selectors/FilterSelector.h>
#include <tracking/trackFindingCDC/collectors/adders/StereoHitTrackAdder.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>
#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitFilterFactory.h>
#include <tracking/trackFindingCDC/utilities/HitComperator.h>

#include <tracking/trackFindingCDC/hough/z0_tanLambda/HitZ0TanLambdaLegendre.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;
    class CDCTrack;

    /**
     * Complex findlet for finding stereo hits from monopoles to a list of cdc tracks.
     *
     * I guess uses collector framework and a quadtree search for quadratic tracks from IP
     */
    class MonopoleStereoHitFinder : public Findlet<CDCWireHit&, CDCTrack&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit&, CDCTrack&>;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      MonopoleStereoHitFinder();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Signal the beginning of a new event
      void beginEvent() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Generates the segment from wire hits
      void apply(std::vector<CDCWireHit>& inputWireHits, std::vector<CDCTrack>& tracks) final;

    private:
      // Findlets
      /// Create RL wire hits out of the wire hits
      RLTaggedWireHitCreator m_rlWireHitCreator;
      /// Find matching hits to a track
      StereoHitTrackQuadTreeMatcher<HitZ0TanLambdaLegendre> m_matcher;
      /// Filter for the Stereo Hits added to the track
      FilterSelector<CDCTrack, CDCRLWireHit, ChooseableFilter<StereoHitFilterFactory>> m_filterSelector;
      /// Select only those where the relation is unique (or the best one in those groups)
      SingleMatchSelector<CDCTrack, CDCRLWireHit, HitComperator> m_singleMatchSelector;
      /// Add the hits to the tracks
      StereoHitTrackAdder m_adder;
      /// Fit the tracks after creation
      TrackSZFitter m_szFitter;

      // Object pools
      /// Vector holding all possible wire hits with all possible RL combinations
      std::vector<CDCRLWireHit> m_rlTaggedWireHits;
      /// Vector of relations between tracks and hits
      std::vector<WeightedRelation<CDCTrack, const CDCRLWireHit>> m_relations;
    };
  }
}
