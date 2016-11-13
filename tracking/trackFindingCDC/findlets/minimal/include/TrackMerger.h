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

#include <framework/core/ModuleParamList.h>

#include <vector>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {

    /// Merges tracks in the same super cluster by linking paths of tracks in a cellular automaton
    template <class TrackRelationFilter>
    class TrackMerger : public Findlet<const CDCTrack, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCTrack, CDCTrack>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      TrackMerger()
      {
        this->addProcessingSignalListener(&m_trackRelationFilter);
      }

      /// Short description of the findlet
      std::string getDescription() final {
        return "Merges tracks by extraction of track paths in a cellular automaton.";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
        m_trackRelationFilter.exposeParameters(moduleParamList, prefix);
      }

    public:
      /// Main algorithm
      void
      apply(const std::vector<CDCTrack>& inputTracks, std::vector<CDCTrack>& outputTracks) final {
        m_trackRelations.clear();
        WeightedNeighborhood<const CDCTrack>::appendUsing(m_trackRelationFilter,
        inputTracks,
        m_trackRelations);
        WeightedNeighborhood<const CDCTrack> trackNeighborhood(m_trackRelations);

        m_trackPaths.clear();
        m_cellularPathFinder.apply(inputTracks, trackNeighborhood, m_trackPaths);

        for (const std::vector<const CDCTrack*>& trackPath : m_trackPaths)
        {
          outputTracks.push_back(CDCTrack::condense(trackPath));
        }
      }

    private:
      /// Wire hit neighborhood relation filter
      TrackRelationFilter m_trackRelationFilter;

      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCTrack> m_cellularPathFinder;

      /// Memory for the relations between tracks to be followed on merge
      std::vector<WeightedRelation<const CDCTrack> > m_trackRelations;

      /// Memory for the track paths generated from the graph.
      std::vector< std::vector<const CDCTrack*> > m_trackPaths;
    };
  }
}
