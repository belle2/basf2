/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/minimal/WeightedRelationCreator.h>

#include <tracking/trackFindingCDC/filters/trackRelation/ChooseableTrackRelationFilter.h>

#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>

#include <vector>
#include <string>

namespace Belle2 {


  namespace TrackFindingCDC {
    class CDCTrack;

    /// Links tracks based on a filter criterion
    class TrackLinker : public Findlet<const CDCTrack, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCTrack, CDCTrack>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      TrackLinker();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    public:
      /// Main algorithm
      void apply(const std::vector<CDCTrack>& inputTracks, std::vector<CDCTrack>& outputTracks) final;

    private:
      /// Creator of the track relations for linking
      WeightedRelationCreator<const CDCTrack, ChooseableTrackRelationFilter> m_trackRelationCreator;

      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCTrack> m_cellularPathFinder;

      /// Memory for the relations between tracks to be followed on linking
      std::vector<WeightedRelation<const CDCTrack> > m_trackRelations;

      /// Memory for the track paths generated from the graph.
      std::vector<Path<const CDCTrack>> m_trackPaths;
    };
  }
}
