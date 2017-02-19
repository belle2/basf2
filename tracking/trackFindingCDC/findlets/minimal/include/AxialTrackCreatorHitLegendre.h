/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtreetools/QuadTreePassCounter.h>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCWireHit;
    enum class LegendreFindingPass;
    class QuadTreeParameters;

    /**
     * Generates axial tracks from hit using special leaf postprocessing.
     *
     * Implements Legendre transformation of the drift time circles.
     * This is a module, performing tracking in the CDC. It is based on the paper
     * "Implementation of the Legendre Transform for track segment reconstruction in drift tube chambers"
     * by T. Alexopoulus, et al. NIM A592 456-462 (2008).
     */
    class AxialTrackCreatorHitLegendre : public Findlet<const CDCWireHit* const, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit* const, CDCTrack>;

    public:
      /// Constructor
      AxialTrackCreatorHitLegendre();

      /// Constructor from a pass key
      explicit AxialTrackCreatorHitLegendre(LegendreFindingPass pass);

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Initialisation before the event processing starts
      void initialize() final;

      /// Execute one pass over a quad tree
      void apply(const std::vector<const CDCWireHit*>& axialWireHits,
                 std::vector<CDCTrack>& tracks) final;

    private:
      /**
       * Performs quadtree search
       * @param lmdInterface lambda interface to operate with QuadTree which contains possible track
       * @param parameters pass-dependent parameters of the QuadTree search
       * @param qtProcessor reference to the AxialHitQuadTreeProcessor instance
       */
      void doTreeTrackFinding(AxialHitQuadTreeProcessor::CandidateProcessorLambda& lmdInterface,
                              QuadTreeParameters& parameters,
                              AxialHitQuadTreeProcessor& qtProcessor);

    private: // Parameters
      /// Maximum Level of FastHough - fixed as the behaviour for other values is not well defined
      static const int m_param_maxLevel = 12;

      /// The pass key for lookup of the parameters for this pass
      LegendreFindingPass m_pass = LegendreFindingPass::NonCurlers;

      /// Parameter to define multiplier for hits threshold for the next quadtree iteration
      const double m_param_stepScale = 0.75;

      /// Parameter to define minimal threshold of hit
      const int m_param_threshold = 10;
    };
  }
}
