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
  StoreArray<CDCDedxTrack> tracks;

  // Collector object access
  auto& means = getObject<TH1F>("means");
  auto& tree = getObject<TTree>("tree");

  // SPECIAL CORRECTION FOR COSMICS -> REMOVE MOMENTUM DEPENDENCE
  const float momcorr[50] = {
    0.936667, 0.791667, 0.763456, 0.755219, 0.758876,
    0.762439, 0.769009, 0.776787, 0.783874, 0.791462,
    0.796567, 0.80445,  0.809177, 0.815605, 0.817414,
    0.822127, 0.828355, 0.83215,  0.832959, 0.833546,
    0.840324, 0.844323, 0.847539, 0.849506, 0.850848,
    0.852272, 0.854783, 0.853612, 0.861432, 0.859428,
    0.859533, 0.862021, 0.865721, 0.868412, 0.868954,
    0.872075, 0.872732, 0.872475, 0.872152, 0.876957,
    0.87419,  0.875742, 0.874523, 0.878218, 0.873543,
    0.881054, 0.874919, 0.877849, 0.886954, 0.882283
  };

  for (auto track : tracks) {

    // clean up cuts -> ONLY FOR COSMICS
    if (track.getMomentum() < 10.0 or track.getNLayerHits() < 43 or track.getNLayerHits() > 64)
      continue;

    // SPECIAL CORRECTION FOR COSMICS -> REMOVE MOMENTUM DEPENDENCE
    int ibinm = 5.0 * track.getMomentum();
    if (ibinm < 0) ibinm = 0;
    if (ibinm > 49) ibinm = 49;

    // Make sure to remove all the data in vectors from the previous track
    m_wire.clear();
    m_layer.clear();
    m_doca.clear();
    m_enta.clear();
    m_dedxhit.clear();

    // Simple numbers don't need to be cleared
    m_dedx = track.getTruncatedMean() / momcorr[ibinm]; // <---- ONLY FOR COSMICS
    m_costh = track.getCosTheta();
    m_nhits = track.size();

    if (m_nhits > m_maxNumHits) continue;
    for (int i = 0; i < m_nhits; ++i) {
      m_wire.push_back(track.getWire(i));
      m_layer.push_back(track.getHitLayer(i));
      m_doca.push_back(track.getDoca(i));
      m_enta.push_back(track.getEnta(i));
      m_dedxhit.push_back(track.getDedx(i) / momcorr[ibinm]); // <---- ONLY FOR COSMICS
    }

    // Track information filled
    tree.Fill();
    means.Fill(m_dedx / momcorr[ibinm]); // <---- ONLY FOR COSMICS
  }
}
