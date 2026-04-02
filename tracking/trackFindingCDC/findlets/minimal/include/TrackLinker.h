/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>
#include <tracking/trackingUtilities/findlets/minimal/WeightedRelationCreator.h>

#include <tracking/trackFindingCDC/filters/trackRelation/ChooseableTrackRelationFilter.h>

#include <tracking/trackingUtilities/ca/MultipassCellularPathFinder.h>

#include <vector>
#include <string>

namespace Belle2 {


  namespace TrackingUtilities {
    class CDCTrack;
  }

  namespace TrackFindingCDC {

    /// Links tracks based on a filter criterion
    class TrackLinker : public TrackingUtilities::Findlet<const TrackingUtilities::CDCTrack, TrackingUtilities::CDCTrack> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::CDCTrack, TrackingUtilities::CDCTrack>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      TrackLinker();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    public:
      /// Main algorithm
      void apply(const std::vector<TrackingUtilities::CDCTrack>& inputTracks,
                 std::vector<TrackingUtilities::CDCTrack>& outputTracks) final;

    private:
      /// Creator of the track relations for linking
      TrackingUtilities::WeightedRelationCreator<const TrackingUtilities::CDCTrack, TrackFindingCDC::ChooseableTrackRelationFilter>
      m_trackRelationCreator;

      /// Instance of the cellular automaton path finder
      TrackingUtilities::MultipassCellularPathFinder<const TrackingUtilities::CDCTrack> m_cellularPathFinder;

      /// Memory for the relations between tracks to be followed on linking
      std::vector<TrackingUtilities::WeightedRelation<const TrackingUtilities::CDCTrack> > m_trackRelations;

      /// Memory for the track paths generated from the graph.
      std::vector<TrackingUtilities::Path<const TrackingUtilities::CDCTrack>> m_trackPaths;
    };
  }
}
