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
// #include <framework/datastore/StoreArray.h>

#include <tracking/datcon/optimizedDATCON/findlets/RelationCreator.dcl.h>
#include <tracking/datcon/optimizedDATCON/filters/relations/ChooseableRelationFilter.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <string>
#include <vector>

#include <TH1D.h>
#include <TH2D.h>
#include <TFile.h>


namespace Belle2 {
  class HitData;
  class SpacePoint;
  class SpacePointTrackCand;
  class VxdID;

  class ModuleParamList;

  /// Findlet for rejecting wrong SpacePointTrackCands and for removing bad hits.
  class RawTrackCandCleaner : public TrackFindingCDC::Findlet<std::vector<HitData*>, SpacePointTrackCand> {
    /// Parent class
    using Super =  TrackFindingCDC::Findlet<std::vector<HitData*>, SpacePointTrackCand>;

  public:
    /// Find intercepts in the 2D Hough space
    RawTrackCandCleaner();

    /// Default destructor
    ~RawTrackCandCleaner();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
//     void initialize() override;

    /// Begin the event and reset containers
//     void beginEvent() override;

    /// End run and write Root file
    void endRun() override;

    /// Reject bad SpacePointTrackCands and bad hits inside the remaining
    void apply(std::vector<std::vector<HitData*>>& rawTrackCandidates, std::vector<SpacePointTrackCand>& trackCandidates) override;

  private:

    /// the current track candidate
//     std::vector<const SpacePoint*> m_currentTrackCandidate;

    /// vector containing track candidates, consisting of the found intersection values in the Hough Space
//     std::vector<SpacePointTrackCand> m_prunedTrackCandidates;

    std::vector<TrackFindingCDC::WeightedRelation<HitData>> m_relations;

    RelationCreator<HitData, ChooseableRelationFilter> m_relationCreator;


    /// ROOT histograms for debugging. Will be deleted when optimization and debugging is done.
    /// ROOT file name
    TFile* m_rootFile;

    TH1D* m_nRelationsPerRawTrackCand;
    TH1D* m_nRelationsPerEvent;
    TH2D* m_nRelationsVsRawTrackCand;
    TH2D* m_nRelationsVsRawTrackCandSize;

    void initializeHists();

  };
}
