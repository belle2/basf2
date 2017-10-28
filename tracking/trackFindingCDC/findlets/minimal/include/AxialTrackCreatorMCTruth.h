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
    class CDCTrack;
    class CDCWireHit;

    /// Findlet that generates tracks from wire hits using the mc truth information.
    class AxialTrackCreatorMCTruth : public Findlet<const CDCWireHit, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit, CDCTrack>;

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
      void apply(const std::vector<CDCWireHit>& inputWireHits,
                 std::vector<CDCTrack>& outputAxialTracks) final;

    private:
      /// Parameter : Setup the drift length as it can be estimated from two dimensional information
      bool m_param_reconstructedDriftLength = true;

      /// Parameter : Switch to reconstruct the positions in the tracks immitating the legendre finder.
      bool m_param_reconstructedPositions = false;

      /// Parameter : Fit the track instead of forwarding the mc truth information
      bool m_param_fit = false;

      /// Parameter : Cut tracks after the last layer of the CDC has been reached, assuming the tracks left the CDC
      bool m_param_useOnlyBeforeTOP = false;
    };
  }
}
