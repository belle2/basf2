/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/datcon/optimizedDATCON/findlets/RelationCreator.h>
#include <tracking/datcon/optimizedDATCON/findlets/DATCONTreeSearcher.dcl.h>
#include <tracking/datcon/optimizedDATCON/findlets/TrackCandidateResultRefiner.h>

#include <tracking/datcon/optimizedDATCON/filters/relations/ChooseableRelationFilter.h>
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/ChooseablePathFilter.h>


#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <string>
#include <vector>

#include <TH1D.h>
#include <TH2D.h>
#include <TFile.h>


namespace Belle2 {
  class SpacePointTrackCand;
  class VxdID;

  class ModuleParamList;

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

    /// End run and write Root file
    void endRun() override;

    /// Reject bad SpacePointTrackCands and bad hits inside the remaining
    void apply(std::vector<std::vector<AHit*>>& rawTrackCandidates, std::vector<SpacePointTrackCand>& trackCandidates) override;

  private:

    /// create relations between the hits in each raw track candidate
    RelationCreator<AHit, ChooseableRelationFilter> m_relationCreator;

    /// perform a tree search using a cellular automaton for all the hits and relations of each raw track candidate
    DATCONTreeSearcher<AHit, ChooseablePathFilter, Result> m_treeSearcher;

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
    uint m_param_maxRelations = 200;


    /// ROOT histograms for debugging. Will be deleted when optimization and debugging is done.
    /// ROOT file name
    std::string m_param_rootFileName;

    TFile* m_rootFile;

    TH1D* m_nRawTrackCandsPerEvent;
    TH1D* m_nRelationsPerRawTrackCand;
    TH1D* m_nRelationsPerEvent;
    TH1D* m_nResultsPerRawTrackCand;
    TH1D* m_nResultsPerEvent;
    TH1D* m_nResultSize;

    TH2D* m_nRelationsVsRawTrackCand;
    TH2D* m_nRelationsVsRawTrackCandSize;
    TH2D* m_nResultsPerRawTCvsnRelationsPerRawTC;
    TH2D* m_nResultsPerRawTCvsRawTCSize;

    TH1D* m_resultQualityEstimator;
    TH2D* m_resultQualityEstimatorvsResultSize;

    TH1D* m_nPrunedResultsPerRawTrackCand;
    TH1D* m_nPrunedResultsPerEvent;
    TH1D* m_nPrunedResultSize;

    TH2D* m_nPrunedResultsPerRawTCvsRawTCSize;

    TH1D* m_prunedResultQualityEstimator;
    TH2D* m_prunedResultQualityEstimatorvsResultSize;


    TH1D* m_nActivePrunedResultsPerRawTrackCand;
    TH1D* m_nActivePrunedResultsPerEvent;
    TH1D* m_nActivePrunedResultSize;

    TH2D* m_nActivePrunedResultsPerRawTCvsRawTCSize;

    TH1D* m_ActivePrunedResultQualityEstimator;
    TH2D* m_ActivePrunedResultQualityEstimatorvsResultSize;

    void initializeHists();
  };
}
