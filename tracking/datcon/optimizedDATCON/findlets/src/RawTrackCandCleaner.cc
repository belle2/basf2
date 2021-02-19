/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/findlets/RawTrackCandCleaner.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>

#include <tracking/datcon/optimizedDATCON/findlets/RelationCreator.icc.h>

#include <tracking/trackFindingCDC/filters/base/RelationFilterUtil.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <iostream>

using namespace Belle2;
using namespace TrackFindingCDC;


RawTrackCandCleaner::~RawTrackCandCleaner() = default;

RawTrackCandCleaner::RawTrackCandCleaner() : Super()
{
  Super::addProcessingSignalListener(&m_relationCreator);

  initializeHists();
}

void RawTrackCandCleaner::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  m_relationCreator.exposeParameters(moduleParamList, prefix);
}

// void RawTrackCandCleaner::initialize()
// {
//   Super::initialize();
// }

// void RawTrackCandCleaner::beginEvent()
// {
//      Super::beginEvent();
// //   m_prunedTrackCandidates.clear();
// //   m_relations.clear();
// }

void RawTrackCandCleaner::apply(std::vector<std::vector<HitData*>>& rawTrackCandidates,
                                std::vector<SpacePointTrackCand>& trackCandidates)
{
  m_relations.reserve(8192);

  uint totalRelationsPerEvent = 0;
  uint counter = 0;
  for (auto& rawTrackCand : rawTrackCandidates) {
    m_relations.clear();
    m_relationCreator.apply(rawTrackCand, m_relations);
//     B2INFO("m_relations.size(): " << m_relations.size());
    totalRelationsPerEvent += m_relations.size();
    m_nRelationsPerRawTrackCand->Fill(m_relations.size());
    m_nRelationsVsRawTrackCand->Fill(counter, m_relations.size());
    m_nRelationsVsRawTrackCandSize->Fill(rawTrackCand.size(), m_relations.size());
    counter++;
  }
  m_nRelationsPerEvent->Fill(totalRelationsPerEvent);


}


void RawTrackCandCleaner::initializeHists()
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


void RawTrackCandCleaner::endRun()
{
  if (m_rootFile) {
    m_rootFile->Write();
    m_rootFile->Close();
  }
}
