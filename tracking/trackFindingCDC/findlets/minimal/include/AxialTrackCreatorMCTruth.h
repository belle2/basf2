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
    class CDCTrack;
    class CDCWireHit;
  }

  namespace TrackFindingCDC {

    /// Findlet that generates tracks from wire hits using the mc truth information.
    class AxialTrackCreatorMCTruth : public
      TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHit, TrackingUtilities::CDCTrack> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHit, TrackingUtilities::CDCTrack>;

    public:
      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Short description of the findlet
      std::string getDescription() final;

      /// Initialize the Module before event processing
      void initialize() final;

      /// Start processing the current event
      void beginEvent() final;

      /// Main function of the track finding by the cellular automaton.
      void apply(const std::vector<TrackingUtilities::CDCWireHit>& inputWireHits,
                 std::vector<TrackingUtilities::CDCTrack>& outputAxialTracks) final;

    private:
      /// Parameter : Setup the drift length as it can be estimated from two dimensional information
      bool m_param_reconstructedDriftLength = true;

      /// Parameter : Switch to reconstruct the positions in the tracks imitating the Legendre finder.
      bool m_param_reconstructedPositions = false;

      /// Parameter : Fit the track instead of forwarding the MC truth information
      bool m_param_fit = false;

      /// Parameter : Cut tracks after the last layer of the CDC has been reached, assuming the tracks left the CDC
      bool m_param_useOnlyBeforeTOP = false;
    };
  }
}
