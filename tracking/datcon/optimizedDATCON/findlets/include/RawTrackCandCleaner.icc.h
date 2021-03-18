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

  initializeHists();
}

template<class AHit>
void RawTrackCandCleaner<AHit>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  m_relationCreator.exposeParameters(moduleParamList, prefix);
  m_treeSearcher.exposeParameters(moduleParamList, prefix);
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
  m_relations.reserve(8192);
  m_results.reserve(64);

  uint totalRelationsPerEvent = 0;
  uint counter = 0;
  for (auto& rawTrackCand : rawTrackCandidates) {
    m_relations.clear();
    m_results.clear();
    m_relationCreator.apply(rawTrackCand, m_relations);
//     B2INFO("m_relations.size(): " << m_relations.size());
    totalRelationsPerEvent += m_relations.size();
    m_nRelationsPerRawTrackCand->Fill(m_relations.size());
    m_nRelationsVsRawTrackCand->Fill(counter, m_relations.size());
    m_nRelationsVsRawTrackCandSize->Fill(rawTrackCand.size(), m_relations.size());
    counter++;

    if (m_relations.size() > 2000) {
      B2WARNING("Aborting because number of relations is above 2000 (exact number: " << m_relations.size() << ") in event: " << m_nEvent);
      continue;
    }

    m_treeSearcher.apply(rawTrackCand, m_relations, m_results);

    for (const std::vector<TrackFindingCDC::WithWeight<const AHit*>>& result : m_results) {
      std::vector<const SpacePoint*> spacePointsInResult;
      spacePointsInResult.reserve(result.size());
      for (const TrackFindingCDC::WithWeight<const AHit*>& hit : result) {
        spacePointsInResult.emplace_back(hit->getHit());
      }
      trackCandidates.emplace_back(spacePointsInResult);
    }

  }
  m_nRelationsPerEvent->Fill(totalRelationsPerEvent);
  m_nEvent++;

//   B2INFO("Event number: " << ++m_nEvent << " with nTrackCands: " << rawTrackCandidates.size() << " and total number of relations: " << totalRelationsPerEvent);

}


template<class AHit>
void RawTrackCandCleaner<AHit>::initializeHists()
{
  m_rootFile = new TFile("relationStats.root", "RECREATE");
  m_rootFile->cd();
  m_nRelationsPerRawTrackCand = new TH1D("nRelationsPerRawTrackCand", "Relations per RawTC;Relations per RawTC;count", 1000, 0, 1000);
  m_nRelationsPerEvent = new TH1D("nRelationsPerEvent", "Relations per event;Relations per event;count", 2000, 0, 20000);
  m_nRelationsVsRawTrackCand = new TH2D("nRelationsVsRawTrackCand", "Relations per RawTC;Relations per RawTC;count", 100, 0, 100,
                                        1000, 0, 1000);
  m_nRelationsVsRawTrackCandSize = new TH2D("nRelationsVsRawTrackCandSize", "Relations vs RawTC size;RawTC size;Relations per RawTC",
                                            200, 0, 200, 1000, 0, 1000);
}


template<class AHit>
void RawTrackCandCleaner<AHit>::endRun()
{
  if (m_rootFile) {
    m_rootFile->Write();
    m_rootFile->Close();
  }
}
