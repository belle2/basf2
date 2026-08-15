/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackingUtilities/eventdata/tracks/CDCTrack.h>

#include <tracking/trackingUtilities/ca/MultipassCellularPathFinder.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCTrack;
    class CDCSegment3D;
  }
  namespace TrackFindingCDC {

    /// Combines two sets of tracks to one final set by merging tracks that have large overlaps
    class TrackCombiner : public
      TrackingUtilities::Findlet<const TrackingUtilities::CDCTrack, const TrackingUtilities::CDCTrack, TrackingUtilities::CDCTrack> {

    private:
      /// Type of the base class
      using Super =
        TrackingUtilities::Findlet<const TrackingUtilities::CDCTrack, const TrackingUtilities::CDCTrack, TrackingUtilities::CDCTrack>;

    public:
      /// Default constructor
      TrackCombiner();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    public:
      /// Main algorithm
      void apply(const std::vector<TrackingUtilities::CDCTrack>& inputTracks,
                 const std::vector<TrackingUtilities::CDCTrack>& secondInputTracks,
                 std::vector<TrackingUtilities::CDCTrack>& tracks) final;

    private:
      /// Parameter : Activate the identification of common segments
      bool m_param_identifyCommonSegments = false;

    private:
      /// Instance of the cellular automaton path finder
      TrackingUtilities::MultipassCellularPathFinder<const TrackingUtilities::CDCSegment3D> m_cellularPathFinder;
    };
  }
}
