/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCWireHit;

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
      /**
       *  Pass keys for the different sets of predefined parameters for a pass if legendre search
       *  Note: Naming copied from original location. Does not actually match with the associated
       *  parameters.
       */
      enum class EPass {
        /// Pass corresponds to High-pt track finding and more deeper quadtree
        NonCurlers,
        /// Pass corresponds to High-pt track finding and more rough quadtree
        NonCurlersWithIncreasingThreshold,
        /// Pass corresponds to full pt range and even more rough quadtree
        /// (non-ip tracks, tracks with energy losses etc)
        FullRange,
        /// Pass corresponds to very High-pt track and very rough quadtree
        /// (monopole tracks, including those with low hit count)
        Straight
      };

    public:
      /// Constructor
      AxialTrackCreatorHitLegendre();

      /// Constructor from a pass key
      explicit AxialTrackCreatorHitLegendre(EPass pass);

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
      /// Method to create QTProcessor that performs the search
      std::unique_ptr<AxialHitQuadTreeProcessor> constructQTProcessor(EPass pass);

      /// lambda function used for postprocessing
      using CandidateReceiver = AxialHitQuadTreeProcessor::CandidateReceiver;
      /**
       * Performs quadtree search
       * @param candidateReceiver lambda interface process found leaves in the legendre search
       * @param qtProcessor reference to the AxialHitQuadTreeProcessor instance
       */
      void executeRelaxation(const CandidateReceiver& candidateReceiver,
                             AxialHitQuadTreeProcessor& qtProcessor);

    private: // Parameters
      /// The pass key for lookup of the parameters for this pass
      EPass m_pass = EPass::NonCurlers;

      /// Parameter to define multiplier for hits threshold for the next quadtree iteration
      const double m_param_stepScale = 0.75;

      /// Parameter to define minimal threshold of hit
      int m_param_minNHits = 10;

      /// Parameter to define precision of quadtree search in case of straight pass
      double m_param_precision = 0.00000001;
    };
  }
}
