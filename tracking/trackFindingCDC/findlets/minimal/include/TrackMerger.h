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

#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>
#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <vector>
#include <iterator>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Merges tracks in the same super cluster by linking paths of tracks in a cellular automaton
    template<class TrackRelationFilter>
    class TrackMerger:
      public Findlet<const CDCTrack, CDCTrack> {

    private:
      /// Type of the base class
      typedef Findlet<const CDCTrack, CDCTrack> Super;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      TrackMerger()
      {
        addProcessingSignalListener(&m_trackRelationFilter);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Merges tracks by extraction of track paths in a cellular automaton.";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override final
      {
        m_trackRelationFilter.exposeParameters(moduleParamList, prefix);
      }

    public:
      /// Main algorithm
      virtual void apply(const std::vector<CDCTrack>& inputTracks,
                         std::vector<CDCTrack>& outputTracks) override final
      {
        m_symmetricTracks.clear();
        m_symmetricTracks.reserve(2 * inputTracks.size());

        for (const CDCTrack& track : inputTracks) {
          m_symmetricTracks.push_back(track);
          m_symmetricTracks.push_back(track.reversed());
        }

        m_trackNeighborhood.clear();
        m_trackNeighborhood.appendUsing(m_trackRelationFilter,
                                        m_symmetricTracks);

        m_trackPaths.clear();
        m_cellularPathFinder.apply(m_symmetricTracks,
                                   m_trackNeighborhood,
                                   m_trackPaths);

        for (const std::vector<const CDCTrack*>& trackPath : m_trackPaths) {
          outputTracks.push_back(CDCTrack::condense(trackPath));
        }
      }

    private:
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCTrack> m_cellularPathFinder;

      /// Memory for the symmetrised tracks
      std::vector<CDCTrack> m_symmetricTracks;

      /// Memory for the wire hit neighborhood in a cluster.
      WeightedNeighborhood<const CDCTrack> m_trackNeighborhood;

      /// Memory for the track paths generated from the graph.
      std::vector< std::vector<const CDCTrack*> > m_trackPaths;

      /// Wire hit neighborhood relation filter
      TrackRelationFilter m_trackRelationFilter;

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2
