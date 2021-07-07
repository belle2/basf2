/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment3D;

    /// Combines two sets of tracks to one final set by merging tracks that have large overlaps
    class TrackCombiner : public Findlet<const CDCTrack, const CDCTrack, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCTrack, const CDCTrack, CDCTrack>;

    public:
      /// Default constructor
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

    private:
      /// Parameter : Activate the identification of common segments
      bool m_param_identifyCommonSegments = false;

    private:
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCSegment3D> m_cellularPathFinder;
    };
  }
}
