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

  initializeHists();

}

void DATCONFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_spacePointLoaderAndPreparer.exposeParameters(moduleParamList, prefix);
  m_interceptFinder.exposeParameters(moduleParamList, prefix);
}

// void DATCONFindlet::initialize()
// {
// //   m_rootFile = new TFile("trackCandStats.root", "RECREATE");
// //   m_rootFile->cd();
// //   m_hitsPerTrackCand = new TH1D("hitsPerTrackCand", "SpacePoints per SPTC", 20, 0, 20);
// //   m_trackCandsPerEvent = new TH1D("trackCandsPerEvent", "SPTCS per event", 100, 0, 100);
//
//   m_storeMCParticles.isOptional();
//
// }

void DATCONFindlet::beginEvent()
{
  Super::beginEvent();

  m_hits.clear();
  m_trackCandidates.clear();

}

void DATCONFindlet::apply()
{
  m_storeMCParticles.isOptional();
  m_storeSVDSpacePoints.isOptional("SVDSpacePoints");

  m_spacePointLoaderAndPreparer.apply(m_hits);
  B2DEBUG(29, "m_hits.size(): " << m_hits.size());

  m_interceptFinder.apply(m_hits, m_trackCandidates);
  B2DEBUG(29, "m_trackCandidates.size: " << m_trackCandidates.size());
  analyseSPTCs();
}

void DATCONFindlet::initializeHists()
{
  m_rootFile = new TFile("trackCandStats.root", "RECREATE");
  m_rootFile->cd();
  m_nMCParticlesPerEvent = new TH1D("nMCParticlesPerEvent", "MCParticles per event;MCParticles per event;count", 50, 0, 50);
  m_nSVDSPsPerEvent = new TH1D("nSVDSPsPerEvent", "SVDSpacePoints per event;SVDSpacePoints per event;count", 2000, 0, 2000);
  m_trackCandsPerEvent = new TH1D("trackCandsPerEvent", "SPTCs per event;SPTCs per event;count", 200, 0, 200);
  m_DiffSPTCsMCParticlesPerEvent = new TH1D("DiffSPTCsMCParticlesPerEvent",
                                            "(nMCParticles - nSPTCs) per event;(nMCParticles - nSPTCs) per event;count", 120, -100, 20);
  m_hitsPerTrackCand = new TH1D("hitsPerTrackCand", "SpacePoints per SPTC;SpacePoints per SPTC;count", 100, 0, 100);

  m_nTrackCandsvsnMCParticles2D = new TH2D("nTrackCandsvsnMCParticles",
                                           "SPTCs vs MCParticles per event;SPCTs per event;MCParticles per event", 200, 0, 200, 50, 0, 50);
  m_nTrackCandsvsnSVDSpacePoints2D = new TH2D("nTrackCandsvsnSVDSpacePoints2D",
                                              "SPTCs vs SVDSpacePoints per event;SVDSpacePoints per event;SPCTs per event", 2000, 0, 2000, 200, 0, 200);
  m_nDiffTrackCandsMCParticlesvsnSVDSpacePoints2D = new TH2D("nDiffTrackCandsMCParticlesvsnSVDSpacePoints2D",
                                                             "(nMCParticles - nSPTCs) vs nSVDSpacePoints;SVDSpacePoints per event;(nMCParticles - nSPTCs) per event",
                                                             2000, 0, 2000, 120, -100, 20);
  m_trackCands2D = new TH2D("trackCands2D", "SPs per SPTC vs SPTCs;SPTC number in this event;SPs per SPTC",
                            100, 0, 100,  100, 0, 100);
}

void DATCONFindlet::analyseSPTCs()
{
  m_nSVDSPsPerEvent->Fill(m_storeSVDSpacePoints.getEntries());
  m_nTrackCandsvsnSVDSpacePoints2D->Fill(m_storeSVDSpacePoints.getEntries(), m_trackCandidates.size());
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
  m_DiffSPTCsMCParticlesPerEvent->Fill(nMCParticles - (int)m_trackCandidates.size());
  m_nDiffTrackCandsMCParticlesvsnSVDSpacePoints2D->Fill(m_storeSVDSpacePoints.getEntries(),
                                                        nMCParticles - (int)m_trackCandidates.size());
  m_nTrackCandsvsnMCParticles2D->Fill(m_trackCandidates.size(), nMCParticles);
  m_trackCandsPerEvent->Fill(m_trackCandidates.size());

  uint i = 0;
  for (auto& tc : m_trackCandidates) {
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
