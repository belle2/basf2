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
  addParam("maxNumHits", m_maxNumHits,
           "Maximum number of hits per track. If there is more than this the track will not be collected. ", int(100));
}

//-----------------------------------------------------------------
//                 Create ROOT objects
//-----------------------------------------------------------------

void CDCDedxElectronCollectorModule::prepare()
{
  StoreArray<CDCDedxTrack>::required();
  StoreArray<Track>::required();
  StoreArray<TrackFitResult>::required();

  // Data object creation
  auto means = new TH1F("means", "CDC dE/dx truncated means", 100, 0, 2);
  auto ttree = new TTree("tree", "Tree with dE/dx information");

  ttree->Branch<double>("dedx", &m_dedx);
  ttree->Branch<double>("costh", &m_costh);

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
  StoreArray<CDCDedxTrack> dedxTracks;

  // Collector object access
  auto& means = getObject<TH1F>("means");
  auto& tree = getObject<TTree>("tree");

  // SPECIAL CORRECTION FOR COSMICS -> REMOVE MOMENTUM DEPENDENCE
  const float momcorr[50] = {
    1.14045,  0.73178,  0.709983, 0.711266, 0.716683,
    0.727419, 0.735754, 0.74534,  0.754149, 0.761252,
    0.768799, 0.77552,  0.780306, 0.786253, 0.79139,
    0.797053, 0.800905, 0.804441, 0.807102, 0.809439,
    0.815215, 0.818581, 0.821492, 0.823083, 0.824502,
    0.828764, 0.830907, 0.831392, 0.832376, 0.833232,
    0.836063, 0.839065, 0.841527, 0.84118,  0.842779,
    0.840801, 0.844476, 0.846664, 0.848733, 0.844318,
    0.84837,  0.850549, 0.852183, 0.851242, 0.856488,
    0.852705, 0.851871, 0.852278, 0.856854, 0.856848
  };

  for (int idedx = 0; idedx < dedxTracks.getEntries(); idedx++) {
    //for (auto track : tracks) {

    CDCDedxTrack* dedxTrack = dedxTracks[idedx];
    const Track* track = dedxTrack->getRelatedFrom<Track>();
    const TrackFitResult* fitResult = track->getTrackFitResult(Const::pion);
    if (!fitResult) {
      B2WARNING("No related fit for this track...");
      continue;
    }

    // clean up cuts -> ONLY FOR COSMICS
    if (dedxTrack->getMomentum() < 1.0 or dedxTrack->getMomentum() > 10.0 or dedxTrack->getNLayerHits() < 41)
      continue;
    if (fitResult->getD0() > 10.0 or (fitResult->getZ0() - 35) > 50.0)
      continue;

    // SPECIAL CORRECTION FOR COSMICS -> REMOVE MOMENTUM DEPENDENCE
    int ibinm = 5.0 * dedxTrack->getMomentum();
    if (ibinm < 0) ibinm = 0;
    if (ibinm > 49) ibinm = 49;

    // Make sure to remove all the data in vectors from the previous track
    m_wire.clear();
    m_layer.clear();
    m_doca.clear();
    m_enta.clear();
    m_dedxhit.clear();

    // Simple numbers don't need to be cleared
    m_dedx = dedxTrack->getTruncatedMean() / 48.0 / momcorr[ibinm]; // <---- ONLY FOR COSMICS
    m_costh = dedxTrack->getCosTheta();
    m_nhits = dedxTrack->size();

    if (m_nhits > m_maxNumHits) continue;
    for (int i = 0; i < m_nhits; ++i) {
      m_wire.push_back(dedxTrack->getWire(i));
      m_layer.push_back(dedxTrack->getHitLayer(i));
      m_doca.push_back(dedxTrack->getDoca(i));
      m_enta.push_back(dedxTrack->getEnta(i));
      m_dedxhit.push_back(dedxTrack->getDedx(i) / 48.0 / momcorr[ibinm]); // <---- ONLY FOR COSMICS
    }

    // Track information filled
    tree.Fill();
    means.Fill(m_dedx); // <---- ONLY FOR COSMICS
  }
}
