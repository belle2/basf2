/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Christian Wessel                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/findlets/DATCONFindlet.h>
#include <tracking/datcon/optimizedDATCON/findlets/RawTrackCandCleaner.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/logging/Logger.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

DATCONFindlet::~DATCONFindlet() = default;

DATCONFindlet::DATCONFindlet()
{
  addProcessingSignalListener(&m_spacePointLoaderAndPreparer);
  addProcessingSignalListener(&m_interceptFinder);
  addProcessingSignalListener(&m_interceptFinderSimple);
  addProcessingSignalListener(&m_rawTCCleaner);
  addProcessingSignalListener(&m_recoTrackStorer);

//   initializeHists();

}

void DATCONFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_spacePointLoaderAndPreparer.exposeParameters(moduleParamList, prefix);
  m_interceptFinder.exposeParameters(moduleParamList, prefix);
  m_interceptFinderSimple.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "simple"));
  m_rawTCCleaner.exposeParameters(moduleParamList, prefix);
  m_recoTrackStorer.exposeParameters(moduleParamList, prefix);

  moduleParamList->getParameter<std::string>("twoHitRelationFilter").setDefaultValue("angleAndTime");
  moduleParamList->getParameter<std::string>("twoHitFilter").setDefaultValue("twoHitVirtualIPQI");
  moduleParamList->getParameter<std::string>("threeHitFilter").setDefaultValue("qualityIndicator");
  moduleParamList->getParameter<std::string>("fourHitFilter").setDefaultValue("qualityIndicator");
  moduleParamList->getParameter<std::string>("fiveHitFilter").setDefaultValue("qualityIndicator");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useSubHoughSpaces"), m_useSubHoughSpaces,
                                "Use Hough spaces working on a subset of hits (=true), or just one Hough space working on all hits at the same time (=false)?",
                                m_useSubHoughSpaces);
}

void DATCONFindlet::beginEvent()
{
  Super::beginEvent();

  m_spacePointVector.clear();
  m_hitDataVector.clear();
  m_rawTrackCandidates.clear();
  m_trackCandidates.clear();

}

void DATCONFindlet::apply()
{
  m_storeMCParticles.isOptional();
  m_storeSVDSpacePoints.isOptional("SVDSpacePoints");

  m_spacePointLoaderAndPreparer.apply(m_spacePointVector, m_hitDataVector);
  B2DEBUG(29, "m_hitDataVector.size(): " << m_hitDataVector.size());

  if (m_useSubHoughSpaces) {
    m_interceptFinder.apply(m_hitDataVector, m_rawTrackCandidates);
  } else {
    m_interceptFinderSimple.apply(m_hitDataVector, m_rawTrackCandidates);
  }
  B2DEBUG(29, "m_rawTrackCandidates.size: " << m_rawTrackCandidates.size());
//   analyseSPTCs();

  m_rawTCCleaner.apply(m_rawTrackCandidates, m_trackCandidates);

  m_recoTrackStorer.apply(m_trackCandidates, m_spacePointVector);
}

void DATCONFindlet::initializeHists()
{
  m_rootFile = new TFile("trackCandStats.root", "RECREATE");
  m_rootFile->cd();
  m_nMCParticlesPerEvent = new TH1D("nMCParticlesPerEvent", "MCParticles per event;MCParticles per event;count", 50, 0, 50);
  m_nSVDSPsPerEvent = new TH1D("nSVDSPsPerEvent", "SVDSpacePoints per event;SVDSpacePoints per event;count", 2000, 0, 2000);
  m_trackCandsPerEvent = new TH1D("trackCandsPerEvent", "RawTrackCands per event;RawTrackCands per event;count", 200, 0, 200);
  m_DiffTrackCandsMCParticlesPerEvent = new TH1D("DiffSPTCsMCParticlesPerEvent",
                                                 "(nMCParticles - nRawTrackCands) per event;(nMCParticles - nRawTrackCands) per event;count", 120, -100, 20);
  m_hitsPerTrackCand = new TH1D("hitsPerTrackCand", "Hits per RawTrackCand;Hits per RawTrackCand;count", 100, 0, 100);

  m_nTrackCandsvsnMCParticles2D = new TH2D("nTrackCandsvsnMCParticles",
                                           "RawTrackCands vs MCParticles per event;RawTrackCands per event;MCParticles per event", 200, 0, 200, 50, 0, 50);
  m_nTrackCandsvsnSVDSpacePoints2D = new TH2D("nTrackCandsvsnSVDSpacePoints2D",
                                              "RawTrackCands vs SVDSpacePoints per event;SVDSpacePoints per event;RawTrackCands per event", 2000, 0, 2000, 200, 0, 200);
  m_nDiffTrackCandsMCParticlesvsnSVDSpacePoints2D = new TH2D("nDiffTrackCandsMCParticlesvsnSVDSpacePoints2D",
                                                             "(nMCParticles - nRawTrackCands) vs nSVDSpacePoints;SVDSpacePoints per event;(nMCParticles - nRawTrackCands) per event",
                                                             2000, 0, 2000, 120, -100, 20);
  m_trackCands2D = new TH2D("trackCands2D",
                            "Hits per RawTrackCand vs RawTrackCands;RawTrackCand number in this event;Hits per RawTrackCand", 100, 0, 100,  100, 0, 100);
}

void DATCONFindlet::analyseSPTCs()
{
  m_nSVDSPsPerEvent->Fill(m_storeSVDSpacePoints.getEntries());
  m_nTrackCandsvsnSVDSpacePoints2D->Fill(m_storeSVDSpacePoints.getEntries(), m_rawTrackCandidates.size());
  int nMCParticles = 0;
  for (auto& mcparticle : m_storeMCParticles) {
    if (mcparticle.hasStatus(MCParticle::c_PrimaryParticle) &&
        mcparticle.hasStatus(MCParticle::c_StableInGenerator) &&
        mcparticle.hasSeenInDetector(Const::SVD) &&
        mcparticle.getCharge() != 0) {
      nMCParticles++;
    }
  }
  m_nMCParticlesPerEvent->Fill(nMCParticles);
  m_DiffTrackCandsMCParticlesPerEvent->Fill(nMCParticles - (int)m_rawTrackCandidates.size());
  m_nDiffTrackCandsMCParticlesvsnSVDSpacePoints2D->Fill(m_storeSVDSpacePoints.getEntries(),
                                                        nMCParticles - (int)m_rawTrackCandidates.size());
  m_nTrackCandsvsnMCParticles2D->Fill(m_rawTrackCandidates.size(), nMCParticles);
  m_trackCandsPerEvent->Fill(m_rawTrackCandidates.size());

  uint i = 0;
  for (auto& tc : m_rawTrackCandidates) {
    m_hitsPerTrackCand->Fill(tc.size());
    m_trackCands2D->Fill(i++, tc.size());
  }
}

void DATCONFindlet::terminate()
{
  if (m_rootFile) {
    m_rootFile->Write();
    m_rootFile->Close();
  }
}
