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

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCTrack;

    /// Combines two sets of tracks to one final set by merging tracks that have large overlaps
    class TrackCombiner : public Findlet<const CDCTrack, const CDCTrack, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCTrack&, CDCTrack&, CDCTrack>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      TrackCombiner();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    public:
      /// Main algorithm
      void apply(const std::vector<CDCTrack>& inputTracks,
                 const std::vector<CDCTrack>& secondInputTracks,
                 std::vector<CDCTrack>& tracks) final;

    };
  }
}
