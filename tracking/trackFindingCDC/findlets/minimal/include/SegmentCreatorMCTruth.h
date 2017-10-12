/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCWireHit;

    /// Findlet that generates segments from wire hits using the mc truth information.
    class SegmentCreatorMCTruth : public Findlet<const CDCWireHit, CDCSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit, CDCSegment2D>;

    public:
      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Short description of the findlet
      std::string getDescription() final;

      /// Initialize the Module before event processing
      void initialize() final;

      /// Start processing the current event
      void beginEvent() final;

      /// Main function of the segment finding by the cellular automaton.
      void apply(const std::vector<CDCWireHit>& inputWireHits,
                 std::vector<CDCSegment2D>& outputSegments) final;

    private:
      /// Parameter : Setup the drift length as it can be estimated from two dimensional information
      bool m_param_reconstructedDriftLength = false;

      /// Parameter : Switch to reconstruct the positions in the segments immitating the facet ca picking up all correct hits.
      bool m_param_reconstructedPositions = false;
    };
  }
}
