/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxElectronCollector/CDCDedxElectronCollectorModule.h>
#include <TTree.h>
#include <TH1F.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCDedxElectronCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxElectronCollectorModule::CDCDedxElectronCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("A collector module for CDC dE/dx electron calibrations");

  // Parameter definitions
  addParam("cleanupCuts", m_cuts, "Boolean to apply cleanup cuts", true);
  addParam("maxNumHits", m_maxNumHits,
           "Maximum number of hits per track. If there is more than this the track will not be collected. ", int(100));
}

//-----------------------------------------------------------------
//                 Create ROOT objects
//-----------------------------------------------------------------

void CDCDedxElectronCollectorModule::prepare()
{
  m_dedxTracks.isRequired();
  m_tracks.isRequired();
  m_trackFitResults.isRequired();

  // Data object creation
  auto means = new TH1F("means", "CDC dE/dx truncated means", 100, 0, 2);
  auto ttree = new TTree("tree", "Tree with dE/dx information");

  ttree->Branch<double>("dedx", &m_dedx);
  ttree->Branch<double>("costh", &m_costh);
  ttree->Branch<double>("p", &m_p);

  ttree->Branch("wire", &m_wire);
  ttree->Branch("layer", &m_layer);
  ttree->Branch("doca", &m_doca);
  ttree->Branch("enta", &m_enta);
  ttree->Branch("dedxhit", &m_dedxhit);

  // Collector object registration
  registerObject<TH1F>("means", means);
  registerObject<TTree>("tree", ttree);
}

//-----------------------------------------------------------------
//                 Fill ROOT objects
//-----------------------------------------------------------------

void CDCDedxElectronCollectorModule::collect()
{

  // Collector object access
  auto means = getObjectPtr<TH1F>("means");
  auto tree = getObjectPtr<TTree>("tree");

  for (int idedx = 0; idedx < m_dedxTracks.getEntries(); idedx++) {
    CDCDedxTrack* dedxTrack = m_dedxTracks[idedx];
    const Track* track = dedxTrack->getRelatedFrom<Track>();
    const TrackFitResult* fitResult = track->getTrackFitResultWithClosestMass(Const::pion);
    if (!fitResult) {
      B2WARNING("No related fit for this track...");
      continue;
    }

    m_p = dedxTrack->getMomentum();
    TVector3 trackMom = fitResult->getMomentum();

    // apply Roy's cuts
    if (m_cuts && (fabs(m_p) >= 10.0 || fabs(m_p) <= 1.0)) continue;
    if (m_cuts && (dedxTrack->getNLayerHits() <= 42 || dedxTrack->getNLayerHits() >= 65)) continue;
    if (m_cuts && (fabs(fitResult->getD0()) >= 5 || fabs(fitResult->getZ0() - 35) >= 50)) continue;

    // Make sure to remove all the data in vectors from the previous track
    m_wire.clear();
    m_layer.clear();
    m_doca.clear();
    m_enta.clear();
    m_dedxhit.clear();

    // Simple numbers don't need to be cleared
    m_dedx = dedxTrack->getDedx();
    m_costh = dedxTrack->getCosTheta();
    m_nhits = dedxTrack->size();

    if (m_nhits > m_maxNumHits) continue;
    for (int i = 0; i < m_nhits; ++i) {
      m_wire.push_back(dedxTrack->getWire(i));
      m_layer.push_back(dedxTrack->getHitLayer(i));
      m_doca.push_back(dedxTrack->getDoca(i));
      m_enta.push_back(dedxTrack->getEnta(i));
      m_dedxhit.push_back(dedxTrack->getDedx(i));
    }

    // Track information filled
    tree->Fill();
    means->Fill(m_dedx);
  }
}
