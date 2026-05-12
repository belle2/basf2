/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackingUtilities {
    class CDCWireHit;
    class CDCSegment2D;
  }

  namespace TrackFindingCDC {

    /// Findlet that generates segments from wire hits using the mc truth information.
    class SegmentCreatorMCTruth : public
      TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHit, TrackingUtilities::CDCSegment2D> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHit, TrackingUtilities::CDCSegment2D>;

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
      void apply(const std::vector<TrackingUtilities::CDCWireHit>& inputWireHits,
                 std::vector<TrackingUtilities::CDCSegment2D>& outputSegments) final;

    private:
      /// Parameter : Setup the drift length as it can be estimated from two dimensional information
      bool m_param_reconstructedDriftLength = false;

      /// Parameter : Switch to reconstruct the positions in the segments imitating the facet ca picking up all correct hits.
      bool m_param_reconstructedPositions = false;
    };
  }
}
