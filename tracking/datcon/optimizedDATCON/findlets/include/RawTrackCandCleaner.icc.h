/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/findlets/RawTrackCandCleaner.dcl.h>

// #include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>

#include <tracking/datcon/optimizedDATCON/entities/HitData.h>

#include <tracking/datcon/optimizedDATCON/filters/relations/LayerRelationFilter.icc.h>
#include <tracking/datcon/optimizedDATCON/findlets/DATCONTreeSearcher.icc.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

#include <tracking/trackFindingCDC/filters/base/RelationFilterUtil.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <iostream>

using namespace Belle2;
using namespace TrackFindingCDC;

template<class AHit>
RawTrackCandCleaner<AHit>::~RawTrackCandCleaner() = default;

template<class AHit>
RawTrackCandCleaner<AHit>::RawTrackCandCleaner() : Super()
{
  Super::addProcessingSignalListener(&m_relationCreator);
  Super::addProcessingSignalListener(&m_treeSearcher);
  Super::addProcessingSignalListener(&m_resultRefiner);

  initializeHists();
}

template<class AHit>
void RawTrackCandCleaner<AHit>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  m_relationCreator.exposeParameters(moduleParamList, prefix);
  m_treeSearcher.exposeParameters(moduleParamList, prefix);
  m_resultRefiner.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maxRelationsCleaner"), m_maxRelations,
                                "Maximum number of relations allowed for entering tree search.",
                                m_maxRelations);
}

template<class AHit>
void RawTrackCandCleaner<AHit>::initialize()
{
  Super::initialize();
}

// template<class AHit>
// void RawTrackCandCleaner<AHit>::beginEvent()
// {
//      Super::beginEvent();
// //   m_prunedTrackCandidates.clear();
// //   m_relations.clear();
// }

template<class AHit>
void RawTrackCandCleaner<AHit>::apply(std::vector<std::vector<AHit*>>& rawTrackCandidates,
                                      std::vector<SpacePointTrackCand>& trackCandidates)
{
//   m_relations.reserve(8192);
  m_relations.reserve(m_maxRelations);
  m_results.reserve(64);

  uint totalRelationsPerEvent = 0;
  uint totalResultsPerEvent = 0;
  uint nPrunedResultsPerEvent = 0;
  uint counter = 0;
  m_nRawTrackCandsPerEvent->Fill(rawTrackCandidates.size());
  for (auto& rawTrackCand : rawTrackCandidates) {
    m_relations.clear();
    m_results.clear();
    m_unfilteredResults.clear();
    m_filteredResults.clear();

    m_relationCreator.apply(rawTrackCand, m_relations);
//     B2INFO("m_relations.size(): " << m_relations.size());
    totalRelationsPerEvent += m_relations.size();
    m_nRelationsPerRawTrackCand->Fill(m_relations.size());
    m_nRelationsVsRawTrackCand->Fill(counter, m_relations.size());
    m_nRelationsVsRawTrackCandSize->Fill(rawTrackCand.size(), m_relations.size());
    counter++;

    if (m_relations.size() > m_maxRelations) {
//       B2WARNING("Aborting because number of relations is above " << m_maxRelations << " (exact number: " << m_relations.size() << ") in event: " << m_nEvent);
      m_relations.clear();
      continue;
    }

    m_treeSearcher.apply(rawTrackCand, m_relations, m_results);

    m_nResultsPerRawTrackCand->Fill(m_results.size());
    totalResultsPerEvent += m_results.size();
    m_nResultsPerRawTCvsnRelationsPerRawTC->Fill(m_relations.size(), m_results.size());
    m_nResultsPerRawTCvsRawTCSize->Fill(rawTrackCand.size(), m_results.size());

    m_unfilteredResults.reserve(m_results.size());
    for (const std::vector<TrackFindingCDC::WithWeight<const AHit*>>& result : m_results) {
      m_nResultSize->Fill(result.size());
      std::vector<const SpacePoint*> spacePointsInResult;
      spacePointsInResult.reserve(result.size());
      for (const TrackFindingCDC::WithWeight<const AHit*>& hit : result) {
        spacePointsInResult.emplace_back(hit->getHit());
      }
      std::sort(spacePointsInResult.begin(), spacePointsInResult.end(), [](const SpacePoint * a, const SpacePoint * b) {
        return
          (a->getVxdID().getLayerNumber() < b->getVxdID().getLayerNumber()) or
          (a->getVxdID().getLayerNumber() == b->getVxdID().getLayerNumber()
           and a->getPosition().Perp() < b->getPosition().Perp());
      });

      m_unfilteredResults.emplace_back(spacePointsInResult);
    }

    m_resultRefiner.apply(m_unfilteredResults, m_filteredResults);


    m_nPrunedResultsPerRawTrackCand->Fill(m_filteredResults.size());
    nPrunedResultsPerEvent += m_filteredResults.size();
    m_nPrunedResultsPerRawTCvsRawTCSize->Fill(rawTrackCand.size(), m_filteredResults.size());

    for (const SpacePointTrackCand& trackCand : m_unfilteredResults) {
      m_resultQualityEstimator->Fill(trackCand.getQualityIndicator());
      m_resultQualityEstimatorvsResultSize->Fill(trackCand.getQualityIndicator(), trackCand.getNHits());
    }

    for (const SpacePointTrackCand& trackCand : m_filteredResults) {
      m_prunedResultQualityEstimator->Fill(trackCand.getQualityIndicator());
      m_prunedResultQualityEstimatorvsResultSize->Fill(trackCand.getQualityIndicator(), trackCand.getNHits());
      m_nPrunedResultSize->Fill(trackCand.getNHits());

      trackCandidates.emplace_back(trackCand);
    }

  }
  m_nRelationsPerEvent->Fill(totalRelationsPerEvent);
  m_nResultsPerEvent->Fill(totalResultsPerEvent);
  m_nPrunedResultsPerEvent->Fill(nPrunedResultsPerEvent);
  m_nEvent++;

//   B2INFO("Event number: " << ++m_nEvent << " with nTrackCands: " << rawTrackCandidates.size() << " and total number of relations: " << totalRelationsPerEvent);

}


template<class AHit>
void RawTrackCandCleaner<AHit>::initializeHists()
{
//   m_rootFile = new TFile("relationStats.root", "RECREATE");
  m_rootFile = new TFile("trackCandAnalysis.root", "RECREATE");
  m_rootFile->cd();
  m_nRawTrackCandsPerEvent = new TH1D("RawTrackCandsPerEvent", "Number of RawTCs per Event;Number of RawTCs;count", 200, 0, 200);
  m_nRelationsPerRawTrackCand = new TH1D("RelationsPerRawTrackCand", "Relations per RawTC;Relations per RawTC;count", 1000, 0, 1000);
  m_nRelationsPerEvent = new TH1D("RelationsPerEvent", "Relations per event;Relations per event;count", 2000, 0, 20000);
  m_nResultsPerRawTrackCand = new TH1D("ResultsPerRawTrackCand", "Number of Results per RawTC;Results per RawTC;count", 2000, 0,
                                       2000);
  m_nResultsPerEvent = new TH1D("ResultsPerEvent", "Number of Results per event;Results per event;count", 2000, 0, 20000);
  m_nResultSize = new TH1D("ResultSize", "Size of each result;Result size;count", 10, 0, 10);

  m_nRelationsVsRawTrackCand = new TH2D("RelationsVsRawTrackCand", "Relations per RawTC;Relations per RawTC;count", 100, 0, 100,
                                        1000, 0, 1000);
  m_nRelationsVsRawTrackCandSize = new TH2D("RelationsVsRawTrackCandSize", "Relations vs RawTC size;RawTC size;Relations per RawTC",
                                            200, 0, 200, 1000, 0, 1000);
  m_nResultsPerRawTCvsnRelationsPerRawTC = new TH2D("ResultsPerRawTCvsnRelationsPerRawTC",
                                                    "Number of Results vs number of Relations per RawTC size;Relations size;Results per RawTC",
                                                    200, 0, 200, 500, 0, 500);
  m_nResultsPerRawTCvsRawTCSize = new TH2D("ResultsPerRawTCvsRawTCSize",
                                           "Number of Results vs RawTC size;RawTC size;Results per RawTC",
                                           200, 0, 200, 500, 0, 500);

  m_resultQualityEstimator = new TH1D("resultQualityEstimator",
                                      "Quality estimator for the single results;Quality estimator (TMath::Prob(chi2, ndf));count", 1000, 0, 1);
  m_resultQualityEstimatorvsResultSize = new TH2D("resultQualityEstimatorvsResultSize",
                                                  "Quality estimator for the single results vs size of the result (=nHits);Quality estimator (TMath::Prob(chi2, ndf));nHits", 1000, 0,
                                                  1, 5, 3, 8);

  m_nPrunedResultsPerRawTrackCand = new TH1D("PrunedResultsPerRawTrackCand",
                                             "Number of pruned results per RawTC;Results per RawTC;count", 2000, 0, 2000);
  m_nPrunedResultsPerEvent = new TH1D("PrunedResultsPerEvent", "Number of pruned results per event;Results per event;count", 2000, 0,
                                      2000);
  m_nPrunedResultSize = new TH1D("PrunedResultSize", "Size of each pruned result;Result size;count", 10, 0, 10);

  m_nPrunedResultsPerRawTCvsRawTCSize = new TH2D("PrunedResultsPerRawTCvsRawTCSize",
                                                 "Number of Results vs RawTC size;RawTC size;Results per RawTC",
                                                 200, 0, 200, 500, 0, 500);

  m_prunedResultQualityEstimator = new TH1D("prunedResultQualityEstimator",
                                            "Quality estimator for the single pruned results;Quality estimator (TMath::Prob(chi2, ndf));count", 1000, 0, 1);
  m_prunedResultQualityEstimatorvsResultSize = new TH2D("prunedResultQualityEstimatorvsResultSize",
                                                        "Quality estimator for the single pruned results vs size of the result (=nHits);Quality estimator (TMath::Prob(chi2, ndf));nHits",
                                                        1000, 0, 1, 5, 3, 8);
}


template<class AHit>
void RawTrackCandCleaner<AHit>::endRun()
{
  if (m_rootFile) {
    m_rootFile->Write();
    m_rootFile->Close();
  }
}
