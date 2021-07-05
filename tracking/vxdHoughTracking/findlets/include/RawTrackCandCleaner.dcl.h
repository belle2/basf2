/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/vxdHoughTracking/findlets/RelationCreator.h>
#include <tracking/vxdHoughTracking/findlets/VXDHoughTrackingTreeSearcher.dcl.h>
#include <tracking/vxdHoughTracking/findlets/TrackCandidateResultRefiner.h>
#include <tracking/vxdHoughTracking/filters/relations/ChooseableRelationFilter.h>
#include <tracking/vxdHoughTracking/filters/pathFilters/ChooseablePathFilter.h>

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamList;
  class SpacePointTrackCand;
  class VxdID;

  namespace vxdHoughTracking {

    /// Findlet for rejecting wrong SpacePointTrackCands and for removing bad hits.
    template<class AHit>
    class RawTrackCandCleaner : public TrackFindingCDC::Findlet<std::vector<AHit*>, SpacePointTrackCand> {
      /// Parent class
      using Super =  TrackFindingCDC::Findlet<std::vector<AHit*>, SpacePointTrackCand>;

      using Result = std::vector<TrackFindingCDC::WithWeight<const AHit*>>;

    public:
      /// Find intercepts in the 2D Hough space
      RawTrackCandCleaner();

      /// Default destructor
      ~RawTrackCandCleaner();

      /// Expose the parameters of the sub findlets.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Create the store arrays
      void initialize() override;

      /// Reject bad SpacePointTrackCands and bad hits inside the remaining
      void apply(std::vector<std::vector<AHit*>>& rawTrackCandidates, std::vector<SpacePointTrackCand>& trackCandidates) override;

    private:

      /// create relations between the hits in each raw track candidate
      RelationCreator<AHit, ChooseableRelationFilter> m_relationCreator;

      /// perform a tree search using a cellular automaton for all the hits and relations of each raw track candidate
      VXDHoughTrackingTreeSearcher<AHit, ChooseablePathFilter, Result> m_treeSearcher;

      /// sort and refine the results for each raw track cand, performing a fit and a basic overlap check
      TrackCandidateResultRefiner m_resultRefiner;

      /// vector containing the relations between the hits in the raw track candidate
      std::vector<TrackFindingCDC::WeightedRelation<AHit>> m_relations;

      /// vector containing track candidates after tree search
      std::vector<Result> m_results;
      /// vector containing unfiltered results, i.e. SpacePointTrackCands containing only the SpacePoins
      /// of the hits in m_results
      std::vector<SpacePointTrackCand> m_unfilteredResults;
      /// vector containing the filtered and pruned results
      /// the filtered results of each raw track candidate will be collected and given back to the caller
      std::vector<SpacePointTrackCand> m_filteredResults;

      /// maximum number of relations that can be created per track candidate
      uint m_param_maxRelations = 2000;

      void initializeHists();
    };

  }
}
