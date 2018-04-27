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
  addParam("momentumCor", m_momCor, "Boolean to apply momentum correction", false);
  addParam("momentumCorFromDB", m_useDBMomCor, "Boolean to apply DB momentum correction", false);
  addParam("scaleCor", m_scaleCor, "Boolean to apply scale correction", false);
  addParam("cosineCor", m_cosineCor, "Boolean to apply cosine correction", false);
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

  auto dbtree = new TTree("dbtree", "Tree with DB objects");

  dbtree->Branch<CDCDedxMomentumCor>("momentumCor", &m_MomentumCor);
  dbtree->Branch<CDCDedxWireGain>("wireGains", &m_WireGains);
  dbtree->Branch<CDCDedxRunGain>("runGain", &m_RunGains);
  dbtree->Branch<CDCDedxCosineCor>("cosineCor", &m_CosineCor);
  dbtree->Branch<CDCDedx2DCell>("twoDCell", &m_2DCell);
  dbtree->Branch<CDCDedx1DCell>("oneDCell", &m_1DCell);

  // Collector object registration
  registerObject<TH1F>("means", means);
  registerObject<TTree>("tree", ttree);
  registerObject<TTree>("dbtree", dbtree);
}

//-----------------------------------------------------------------
//                 Get the DBObjects once per run
//-----------------------------------------------------------------

void CDCDedxElectronCollectorModule::startRun()
{
  // Collector object access
  auto dbtree = getObjectPtr<TTree>("dbtree");

  // get the DB objects
  m_MomentumCor = *m_DBMomentumCor;
  m_WireGains = *m_DBWireGains;
  m_RunGains = *m_DBRunGains;
  m_CosineCor = *m_DBCosineCor;
  //  m_2DCell = *m_DB2DCell;
  m_1DCell = *m_DB1DCell;

  dbtree->Fill();
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
    if (m_cuts && (trackMom.Phi() >= 0 || fitResult->getD0() >= 5 || fabs(fitResult->getZ0() - 35) >= 50)) continue;

    // determine the correction factor, if any
    double correction = 1.0;

    // apply the momentum correction
    if (m_momCor) correction *= m_DBMomentumCor->getMean(fabs(m_p));

    // apply the scale factor
    if (m_scaleCor) correction *= m_DBScaleFactor->getScaleFactor();

    // apply the cosine corection
    double costh = dedxTrack->getCosTheta();
    if (m_cosineCor) correction *= m_DBCosineCor->getMean(costh);

    // don't keep this event if the correction is zero
    if (correction == 0) continue;

    // Make sure to remove all the data in vectors from the previous track
    m_wire.clear();
    m_layer.clear();
    m_doca.clear();
    m_enta.clear();
    m_dedxhit.clear();

    // Simple numbers don't need to be cleared
    m_dedx = dedxTrack->getDedx() / correction;
    m_costh = costh;
    m_nhits = dedxTrack->size();

    if (m_nhits > m_maxNumHits) continue;
    for (int i = 0; i < m_nhits; ++i) {
      m_wire.push_back(dedxTrack->getWire(i));
      m_layer.push_back(dedxTrack->getHitLayer(i));
      m_doca.push_back(dedxTrack->getDoca(i));
      m_enta.push_back(dedxTrack->getEnta(i));
      m_dedxhit.push_back(dedxTrack->getDedx(i) / correction);
    }

    // Track information filled
    tree->Fill();
    means->Fill(m_dedx);
  }
}
