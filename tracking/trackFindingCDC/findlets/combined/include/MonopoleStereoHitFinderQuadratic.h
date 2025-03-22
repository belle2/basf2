/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/findlets/minimal/RLTaggedWireHitCreator.h>
// #include <tracking/trackFindingCDC/findlets/minimal/TrackSZFitter.h>

#include <tracking/trackFindingCDC/collectors/matchers/StereoHitTrackQuadTreeMatcher.h>
#include <tracking/trackingUtilities/collectors/selectors/SingleMatchSelector.h>
#include <tracking/trackingUtilities/collectors/selectors/FilterSelector.h>
#include <tracking/trackFindingCDC/collectors/adders/StereoHitTrackAdder.h>

#include <tracking/trackingUtilities/filters/base/ChooseableFilter.dcl.h>
#include <tracking/trackFindingCDC/filters/stereoHits/StereoHitFilterFactory.h>
#include <tracking/trackingUtilities/utilities/HitComperator.h>

#include <tracking/trackFindingCDC/hough/quadratic/HitQuadraticLegendre.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackInspector.h>

#include <vector>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCTrack;
    class CDCWireHit;
  }
  namespace TrackFindingCDC {

    /**
     * Complex findlet for finding stereo hits from monopoles to a list of cdc tracks.
     *
     * WARNING This findlet is kept here just in case hyperbolic one misbehaves and eats up too much RAM
     * If it doesn't, this one should be removed around release-05 or earlier
     *
     * I guess uses collector framework and a quadtree search for quadratic tracks from IP
     */
    class MonopoleStereoHitFinderQuadratic : public
      TrackingUtilities::Findlet<TrackingUtilities::CDCWireHit&, TrackingUtilities::CDCTrack&> {

    private:
      /// Type of the base class
      using Super = Findlet<TrackingUtilities::CDCWireHit&, TrackingUtilities::CDCTrack&>;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      MonopoleStereoHitFinderQuadratic();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Signal the beginning of a new event
      void beginEvent() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Generates the segment from wire hits
      void apply(std::vector<TrackingUtilities::CDCWireHit>& inputWireHits, std::vector<TrackingUtilities::CDCTrack>& tracks) final;

    private:
      // Findlets
      /// Create RL wire hits out of the wire hits
      RLTaggedWireHitCreator m_rlWireHitCreator;
      /// Find matching hits to a track
      StereoHitTrackQuadTreeMatcher<HitQuadraticLegendre> m_matcher;
      /// Filter for the Stereo Hits added to the track
      TrackingUtilities::FilterSelector<TrackingUtilities::CDCTrack, CDCRLWireHit, TrackingUtilities::ChooseableFilter<StereoHitFilterFactory>>
          m_filterSelector;
      /// Select only those where the relation is unique (or the best one in those groups)
      TrackingUtilities::SingleMatchSelector<TrackingUtilities::CDCTrack, CDCRLWireHit, TrackingUtilities::HitComperator>
      m_singleMatchSelector;
      /// Add the hits to the tracks
      StereoHitTrackAdder m_adder;
      /// Print found tracks if requested in module options
      TrackInspector m_inspector;
      // TODO it would be nice to fit the hits while they are still a CDCTrack object and not wait for the final genfit

      // Object pools
      /// Vector holding all possible wire hits with all possible RL combinations
      std::vector<CDCRLWireHit> m_rlTaggedWireHits;
      /// Vector of relations between tracks and hits
      std::vector<TrackingUtilities::WeightedRelation<TrackingUtilities::CDCTrack, const CDCRLWireHit>> m_relations;
    };
  }
}
