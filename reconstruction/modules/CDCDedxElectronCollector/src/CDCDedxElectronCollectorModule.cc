/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jikumar, jvbennett                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxElectronCollector/CDCDedxElectronCollectorModule.h>
#include <framework/dataobjects/EventMetaData.h>
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
  addParam("fSetEoverP", fSetEoverP, "Set E over p Cut values. ", double(0.2));
  addParam("Iscosth", Iscosth, "true for adding costh tree branch. ", false);
  addParam("Isp", Isp, "true for adding momentum tree branch. ", true);
  addParam("Ischarge", Ischarge, "true for charge dedx tree branch. ", false);
  addParam("Isrun", Isrun, "true for adding run number tree branch. ", false);
  addParam("Iswire", Iswire, "true for adding wires tree branch. ", false);
  addParam("Islayer", Islayer, "true for adding layers tree branch. ", false);
  addParam("Isdoca", Isdoca, "true for adding doca tree branch. ", false);
  addParam("Isenta", Isenta, "true for adding enta tree branch. ", false);
  addParam("IsdocaRS", IsdocaRS, "true for adding doca tree branch. ", false);
  addParam("IsentaRS", IsentaRS, "true for adding enta tree branch. ", false);
  addParam("Isdedxhit", Isdedxhit, "true for adding dedxhit tree branch. ", false);
  addParam("IsBadPhiRej", IsBadPhiRej, "true for removing bad phi tracks and hits ", false);
  addParam("IsRadbhabha", IsRadbhabha, "true for cutting dedx of other track", false);


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
  if (Iscosth)ttree->Branch<double>("costh", &m_costh);
  if (Isp)ttree->Branch<double>("p", &m_p);
  if (Ischarge)ttree->Branch<int>("charge", &m_charge);
  if (Isrun)ttree->Branch<int>("run", &m_run);

  if (Iswire)ttree->Branch("wire", &m_wire);
  if (Islayer)ttree->Branch("layer", &m_layer);
  if (Isdoca)ttree->Branch("doca", &m_doca);
  if (Isenta)ttree->Branch("enta", &m_enta);
  if (IsdocaRS)ttree->Branch("docaRS", &m_docaRS);
  if (IsentaRS)ttree->Branch("entaRS", &m_entaRS);
  if (Isdedxhit)ttree->Branch("dedxhit", &m_dedxhit);

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

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  int run = eventMetaDataPtr->getRun();
  if (Isrun)m_run = run;
  //printf("this run is = %d", run);

  Int_t nTracks = m_dedxTracks.getEntries();

  for (int idedx = 0; idedx < m_dedxTracks.getEntries(); idedx++) {

    CDCDedxTrack* dedxTrack = m_dedxTracks[idedx];

    const Track* track = dedxTrack->getRelatedFrom<Track>();

    const TrackFitResult* fitResult = track->getTrackFitResultWithClosestMass(Const::electron);
    if (!fitResult) {
      B2WARNING("No related fit for this track...");
      continue;
    }

    if (Isp) {
      m_p = dedxTrack->getMomentum();
      TVector3 trackMom = fitResult->getMomentum();
    }

    // apply cleanup cuts
    if (m_cuts && dedxTrack->getNLayerHits() <= 20) continue;

    if (m_cuts && (fabs(fitResult->getD0()) >= 1 || fabs(fitResult->getZ0()) >= 3)) continue;

    ////NEW
    const ECLCluster* eclCluster = track->getRelated<ECLCluster>();
    if (eclCluster and eclCluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
      double TrkEoverP = (eclCluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons)) / (fitResult->getMomentum().Mag());
      if (abs(TrkEoverP - 1) >= fSetEoverP)continue;
      //printf("TrkEoverP = %0.03f\n", TrkEoverP);
    }

    if (IsRadbhabha) {
      if (nTracks == 2) {

        Int_t iOtherdedx = abs(idedx - 1);
        CDCDedxTrack* dedxOtherTrack = m_dedxTracks[iOtherdedx];
        if (!dedxOtherTrack)  continue;

        const Track* Othertrack = dedxOtherTrack->getRelatedFrom<Track>();
        if (!Othertrack)continue;

        const TrackFitResult* mOtherTrack = 0x0;
        mOtherTrack = Othertrack->getTrackFitResultWithClosestMass(Const::electron);
        if (!mOtherTrack)continue;

        double TrkEoverPOther = -2.0;
        const ECLCluster* eclClusterOther = Othertrack->getRelated<ECLCluster>();
        if (!eclClusterOther or !eclClusterOther->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))continue;

        TrkEoverPOther = (eclClusterOther->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons)) / (mOtherTrack->getMomentum().Mag());

        //cutting on EoverP of other track
        if (abs(TrkEoverPOther - 1.0) >= fSetEoverP) {
          //printf("Cut1: TrkEoverPOther = %0.03f\n", TrkEoverPOther);
          continue;
        }
        //cutting on dedx of other track
        if (dedxOtherTrack->getDedxNoSat() <= 0.85 || dedxOtherTrack->getDedxNoSat() >= 1.25) {
          //printf("Cut2: TrkdEdxOther = %0.03f\n", dedxOtherTrack->getDedxNoSat());
          continue;
        }
      }
    }

    if (IsBadPhiRej) {
      double TrkPhi = fitResult->getPhi();
      if (abs(TrkPhi - 1.3) < 0.35)continue;
    }

    // Make sure to remove all the data in vectors from the previous track
    if (Iswire)m_wire.clear();
    if (Islayer)m_layer.clear();
    if (Isdoca)m_doca.clear();
    if (Isenta)m_enta.clear();
    if (IsdocaRS)m_docaRS.clear();
    if (IsentaRS)m_entaRS.clear();
    if (Isdedxhit)m_dedxhit.clear();

    // Simple numbers don't need to be cleared
    // make sure to use the truncated mean without the hadron saturation correction
    m_nhits = dedxTrack->size();
    if (m_nhits > m_maxNumHits) continue;

    m_dedx = dedxTrack->getDedxNoSat();
    if (Iscosth)m_costh = dedxTrack->getCosTheta();
    if (Ischarge)m_charge = fitResult->getChargeSign();

    for (int i = 0; i < m_nhits; ++i) {

      if (m_DBWireGains->getWireGain(dedxTrack->getWire(i)) == 0)continue; //Jake added
      //if (dedxTrack->getPath(i) <= 0.5)continue; //JK
      if (Iswire)m_wire.push_back(dedxTrack->getWire(i));
      if (Islayer) m_layer.push_back(dedxTrack->getHitLayer(i));
      if (Isdoca)m_doca.push_back(dedxTrack->getDoca(i));
      if (Isenta)m_enta.push_back(dedxTrack->getEnta(i));
      if (IsdocaRS)m_docaRS.push_back(dedxTrack->getDocaRS(i) / dedxTrack->getCellHalfWidth(i));
      if (IsentaRS)m_entaRS.push_back(dedxTrack->getEntaRS(i));
      if (Isdedxhit)m_dedxhit.push_back(dedxTrack->getDedx(i));
    }

    // Track information filled
    tree->Fill();
    means->Fill(m_dedx);
  }
}
