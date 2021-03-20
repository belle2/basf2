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

    /// Begin the event and reset containers
//     void beginEvent() override;

    /// End run and write Root file
    void endRun() override;

    /// Reject bad SpacePointTrackCands and bad hits inside the remaining
    void apply(std::vector<std::vector<AHit*>>& rawTrackCandidates, std::vector<SpacePointTrackCand>& trackCandidates) override;

  private:

    RelationCreator<AHit, ChooseableRelationFilter> m_relationCreator;

    DATCONTreeSearcher<AHit, ChooseablePathFilter, Result> m_treeSearcher;

    /// the current track candidate
//     std::vector<const SpacePoint*> m_currentTrackCandidate;

    /// vector containing track candidates, consisting of the found intersection values in the Hough Space
    std::vector<Result> m_results;

    std::vector<TrackFindingCDC::WeightedRelation<AHit>> m_relations;

    uint m_maxRelations = 200;


    /// ROOT histograms for debugging. Will be deleted when optimization and debugging is done.
    /// ROOT file name
    TFile* m_rootFile;

    TH1D* m_nRelationsPerRawTrackCand;
    TH1D* m_nRelationsPerEvent;
    TH1D* m_nResultsPerRawTrackCand;
    TH1D* m_nResultsPerEvent;
    TH1D* m_nResultSize;

    TH2D* m_nRelationsVsRawTrackCand;
    TH2D* m_nRelationsVsRawTrackCandSize;
    TH2D* m_nResultsPerRawTCvsnRelationsPerRawTC;
    TH2D* m_nResultsPerRawTCvsRawTCSize;

    void initializeHists();

    int m_nEvent = 0;

  };
}
