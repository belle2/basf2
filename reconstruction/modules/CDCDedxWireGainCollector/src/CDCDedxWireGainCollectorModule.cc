/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxWireGainCollector/CDCDedxWireGainCollectorModule.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCDedxWireGainCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxWireGainCollectorModule::CDCDedxWireGainCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("A collector module for CDC dE/dx wire gain calibration");

  // Parameter definitions
  addParam("maxNumHits", m_maxNumHits,
           "Maximum number of hits per track. If there is more than this the track will not be collected. ", int(100));
}

//-----------------------------------------------------------------
//                 Create ROOT objects
//-----------------------------------------------------------------

void CDCDedxWireGainCollectorModule::prepare()
{
  StoreArray<CDCDedxTrack>::required();

  // Data object creation
  auto ttree = new TTree("dedxTree", "Tree with dE/dx information");
  ttree->Branch<double>("dedx", &m_dedx);
  ttree->Branch<double>("costh", &m_costh);
  // No longer need to store the nhits as the size of the wire/layer/dedxhit vectors are the same as the number of hits

  ttree->Branch("wire", &m_wire);
  ttree->Branch("layer", &m_layer);
  ttree->Branch("dedxhit", &m_dedxhit);

  // Collector object registration
  registerObject<TTree>("tree", ttree);
}

//-----------------------------------------------------------------
//                 Fill ROOT objects
//-----------------------------------------------------------------

void CDCDedxWireGainCollectorModule::collect()
{
  StoreArray<CDCDedxTrack> tracks;
  // Collector object access
  auto& tree = getObject<TTree>("tree");

  for (auto track : tracks) {
    // Make sure to remove all the data in vectors from the previous track
    m_wire.clear();
    m_layer.clear();
    m_dedxhit.clear();

    // Simple numbers don't need to be cleared
    m_dedx = track.getTruncatedMean();
    m_costh = track.getCosTheta();
    m_nhits = track.getNLayerHits();  // Used in loop below but not saved to TTree

    if (m_nhits > m_maxNumHits) continue;  // Do you want this maximum if you don't 'have' to have it?
    for (int i = 0; i < m_nhits; ++i) {
      m_wire.push_back(track.getWire(i));
      m_layer.push_back(track.getLayer(i));
      m_dedxhit.push_back(track.getDedx(i));
    }

    // Track information filled
    tree.Fill();
  }
}
