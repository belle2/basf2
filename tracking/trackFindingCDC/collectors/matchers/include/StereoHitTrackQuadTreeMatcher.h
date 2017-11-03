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
#include <tracking/trackFindingCDC/collectors/matchers/MatcherInterface.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * A matcher algorithm for using a stereo quad tree for matching rl tagged wire hits
     * to tracks.
     */
    template <class AQuadTree>
    class StereoHitTrackQuadTreeMatcher : public MatcherInterface<CDCTrack, CDCRLWireHit> {

      /// The parent class.
      using Super = MatcherInterface<CDCTrack, CDCRLWireHit>;

    public:
      /// Expose the parameters to the module.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Initialize the filter and the quad tree.
      void initialize() override;

      /// Terminate the filter and the quad tree.
      void terminate() override;

    private:
      /**
       * Create a QuadTree and fill with each unused stereo hit (to be exact: twice for each stereo hit - right and left).
       * The QuadTree has two dimensions: inverse slope in z-direction and z0.
       * Each bin with a high number of items (= stereo hits) in it is stored. Later, the one node with the highest number of items in it is taken
       * and each hit is assigned to the track.
       */
      void match(CDCTrack& track, const std::vector<CDCRLWireHit>& rlWireHits,
                 std::vector<Super::WeightedRelationItem>& relationsForCollector) override;

      /// Use the writeDebugInformation function of the quad tree to write the tree into a root file with a ascending number.
      void writeDebugInformation();

      /// Parameters
      /// Set to false to skip the B2B check (good for curlers).
      bool m_param_checkForB2BTracks = true;
      /// Set to false to skip the in-wire-bound check (good for second stage).
      double m_param_checkForInWireBoundsFactor = 1.0;
      /// Maximum level of the quad tree search.
      unsigned int m_param_quadTreeLevel = 7;
      /// Minimal number of hits a quad tree node must have to be called a found bin
      unsigned int m_param_minimumNumberOfHits = 5;
      /// Set to true to output debug information.
      bool m_param_writeDebugInformation = false;

      /// Store the number of passed calls to the debug function.
      unsigned int m_numberOfPassedDebugCalls = 0;

      /// Quad tree instance
      AQuadTree m_quadTreeInstance;
    };
  }
}
