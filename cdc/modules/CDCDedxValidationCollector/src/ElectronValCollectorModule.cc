/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/modules/CDCDedxValidationCollector/ElectronValCollectorModule.h>

using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ElectronValCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
ElectronValCollectorModule::ElectronValCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("A collector module for CDC dE/dx validation");

  // Parameter definitions
  addParam("cleanupCuts", m_cuts, "boolean to apply cleanup cuts", true);
  addParam("maxHits", m_maxHits, "maximum number of hits per track ", int(100));
  addParam("setEoP", m_setEoP, "Set E over p Cut values. ", double(0.25));
}

//-----------------------------------------------------------------
//                 Create ROOT objects
//-----------------------------------------------------------------

void ElectronValCollectorModule::prepare()
{
  m_trgResult.isOptional();
  m_dedxTracks.isRequired();
  m_tracks.isRequired();
  m_trackFitResults.isRequired();

  // Data object creation
  auto means = new TH1D("means", "CDC dE/dx truncated means", 250, 0, 2.5);

  auto tBhabha = new TTree("tBhabha", "Tree with dE/dx information for electrons");
  auto tRadee = new TTree("tRadee", "Tree with dE/dx information for radiative electrons");

  auto hestats = new TH1I("hestats", "Event Stats", 8, -0.5, 7.5);
  hestats->SetFillColor(kYellow);
  hestats->GetXaxis()->SetBinLabel(1, "all");
  hestats->GetXaxis()->SetBinLabel(2, "notrig");
  hestats->GetXaxis()->SetBinLabel(3, "noskim");
  hestats->GetXaxis()->SetBinLabel(4, "bhabha");
  hestats->GetXaxis()->SetBinLabel(5, "radee");
  hestats->GetXaxis()->SetBinLabel(6, "wrong skim");
  hestats->GetXaxis()->SetBinLabel(7, "unclean");
  hestats->GetXaxis()->SetBinLabel(8, "selected");

  auto htstats = new TH1I("htstats", "track Stats", 7, -0.5, 6.5);
  htstats->SetFillColor(kYellow);
  htstats->GetXaxis()->SetBinLabel(1, "alltrk");
  htstats->GetXaxis()->SetBinLabel(2, "vtx");
  htstats->GetXaxis()->SetBinLabel(3, "inCDC");
  htstats->GetXaxis()->SetBinLabel(4, "whits");
  htstats->GetXaxis()->SetBinLabel(5, "weop");
  htstats->GetXaxis()->SetBinLabel(6, "radee");
  htstats->GetXaxis()->SetBinLabel(7, "selected");

  tRadee->Branch<double>("injtime", &m_injTime);
  tRadee->Branch<double>("injring", &m_injRing);
  tRadee->Branch<double>("costh", &m_costh);
  tRadee->Branch<int>("charge", &m_charge);
  tRadee->Branch<int>("run", &m_run);
  tRadee->Branch<double>("p", &m_p);
  tRadee->Branch<double>("pt", &m_pt);
  tRadee->Branch<double>("dedx", &m_dedx);
  tRadee->Branch("enta", &m_enta);
  tRadee->Branch("entaRS", &m_entaRS);
  tRadee->Branch("layer", &m_layer);
  tRadee->Branch("dedxhit", &m_dedxhit);


  tBhabha->Branch<double>("dedx", &m_dedx);
  tBhabha->Branch<double>("costh", &m_costh);
  tBhabha->Branch<double>("p", &m_p);
  tBhabha->Branch<int>("charge", &m_charge);
  tBhabha->Branch<int>("run", &m_run);
  tBhabha->Branch("wire", &m_wire);
  tBhabha->Branch("dedxhit", &m_dedxhit);

  // Collector object registration
  registerObject<TH1D>("means", means);
  registerObject<TTree>("tRadee", tRadee);
  registerObject<TTree>("tBhabha", tBhabha);

  registerObject<TH1I>("hestats", hestats);
  registerObject<TH1I>("htstats", htstats);
}

//-----------------------------------------------------------------
//                 Fill ROOT objects
//-----------------------------------------------------------------
void ElectronValCollectorModule::collect()
{

  // Retrieve the histogram pointer
  auto hestats = getObjectPtr<TH1I>("hestats");
  if (!hestats) {
    B2ERROR("Failed to retrieve histogram 'hestats'");
    return;
  }
  hestats->Fill(0); // Fill the first bin to indicate processing has started

  // Check if the trigger result is valid
  if (!m_trgResult.isValid()) {
    B2WARNING("SoftwareTriggerResult required to select bhabha/radee event is not found");
    hestats->Fill(1); // Fill bin 2 to indicate missing trigger result
    return;
  }

  // Retrieve the trigger results
  const std::map<std::string, int>& fresults = m_trgResult->getResults();

  // Check for the required trigger identifiers
  if (fresults.find("software_trigger_cut&skim&accept_bhabha") == fresults.end() &&
      fresults.find("software_trigger_cut&skim&accept_radee") == fresults.end()) {
    B2WARNING("Can't find required bhabha/radee trigger identifiers");
    hestats->Fill(2); // Fill bin 3 to indicate missing trigger identifiers
    return;
  }

  // Defining event trigger conditions
  const bool eBhabha = (m_trgResult->getResult("software_trigger_cut&skim&accept_bhabha") == SoftwareTriggerCutResult::c_accept);
  const bool eRadBhabha = (m_trgResult->getResult("software_trigger_cut&skim&accept_radee") == SoftwareTriggerCutResult::c_accept);

  // Handling different event types
  if (eBhabha) {
    B2INFO("Bhabha events");
    hestats->Fill(3);  // Bin 4 (index 3)
  } else if (eRadBhabha) {
    B2INFO("Radiative Bhabha events");
    hestats->Fill(4);  // Bin 5 (index 4)
  } else {
    B2WARNING("Requested event not found: going back");
    hestats->Fill(5);  // Bin 6 (index 5)
    return;  // Exiting the function if event is not found
  }

  // Ensure eventMetaDataPtr is valid
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  if (!eventMetaDataPtr) {
    B2ERROR("Failed to retrieve event metadata");
    return;
  }

  int run = eventMetaDataPtr->getRun();
  m_run = run;

  int nTracks = m_dedxTracks.getEntries();
  if (nTracks >= 4) {
    B2WARNING("too many tracks: unclean bhabha or radee event: " << nTracks);
    hestats->Fill(6);
    return;
  }

  hestats->Fill(7); // Fill bin 8 (index 7) to indicate a valid event

  //Collector object access
  auto tBhabha = getObjectPtr<TTree>("tBhabha");
  auto tRadee = getObjectPtr<TTree>("tRadee");
  auto htstats = getObjectPtr<TH1I>("htstats");
  auto hmeans = getObjectPtr<TH1D>("means");

  for (int idedx = 0; idedx < nTracks; idedx++) {

    CDCDedxTrack* dedxTrack = m_dedxTracks[idedx];
    if (!dedxTrack) {
      B2WARNING("No dedx track: Going back: " << idedx);
      continue;
    }

    const Track* track = dedxTrack->getRelatedFrom<Track>();
    if (!track) {
      B2WARNING("No track: Going back: " << idedx);
      continue;
    }

    const TrackFitResult* fitResult = track->getTrackFitResultWithClosestMass(Const::pion);
    if (!fitResult) {
      B2WARNING("No related fit for this track...");
      continue;
    }

    m_dedx = dedxTrack->getDedxNoSat();
    m_p = dedxTrack->getMomentum();
    m_costh = dedxTrack->getCosTheta();
    m_charge = fitResult->getChargeSign();
    m_pt = fitResult->getTransverseMomentum();
    m_injTime = dedxTrack->getInjectionTime();
    m_injRing = dedxTrack->getInjectionRing();
    m_nhits = dedxTrack->size();

    htstats->Fill(0);

    if (m_cuts) {
      // apply cleanup cuts
      if (fabs(fitResult->getD0()) >= 1.0)continue;
      if (fabs(fitResult->getZ0()) >= 1.0) continue;
      htstats->Fill(1);

      //if outside CDC
      if (m_costh < TMath::Cos(150.0 * TMath::DegToRad()))continue; //-0.866
      if (m_costh > TMath::Cos(17.0 * TMath::DegToRad())) continue; //0.95
      htstats->Fill(2);

      if (m_nhits > m_maxHits) continue;

      //making some cuts based on acceptance
      if (m_costh > -0.55 && m_costh < 0.820) {
        if (dedxTrack->getNLayerHits() < 25)continue; //all CDC layer available here
      } else {
        if (m_costh <= -0.62 || m_costh >= 0.880) {
          if (dedxTrack->getNLayerHits() < 10)continue; //less layer available here
          if (m_costh > 0 && dedxTrack->getNLayerHits() < 13)continue;
        } else {
          if (dedxTrack->getNLayerHits() < 18)continue;
        }
      }
      htstats->Fill(3);

      const ECLCluster* eclCluster = track->getRelated<ECLCluster>();
      if (eclCluster and eclCluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
        double TrkEoverP = (eclCluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons)) / (fitResult->getMomentum().R());
        if (abs(TrkEoverP - 1.0) > m_setEoP)continue;
      }
      htstats->Fill(4);
    }

    //if dealing with radee here (do a safe side cleanup)
    if (eRadBhabha) {
      if (nTracks != 2)continue; //exactly 2 tracks
      bool goodradee = false;
      //checking if dedx of other track is restricted
      //will not do too much as radee is clean enough
      for (int jdedx = 0; jdedx < nTracks; jdedx++) {
        CDCDedxTrack* dedxOtherTrack = m_dedxTracks[abs(jdedx - 1)];
        if (!dedxOtherTrack)continue;
        if (abs(dedxOtherTrack->getDedxNoSat() - 1.0) > 0.25)continue; //loose for uncalibrated
        goodradee = true;
        break;
      }
      if (!goodradee)continue;
      htstats->Fill(5);
    }

    // Make sure to remove all the data in vectors from the previous track
    m_wire.clear();
    m_layer.clear();
    m_dedxhit.clear();
    m_enta.clear();
    m_entaRS.clear();

    for (int i = 0; i < m_nhits; ++i) {
      m_wire.push_back(dedxTrack->getWire(i));
      m_layer.push_back(dedxTrack->getHitLayer(i));
      m_dedxhit.push_back(dedxTrack->getDedx(i));
      m_enta.push_back(dedxTrack->getEnta(i));
      m_entaRS.push_back(dedxTrack->getEntaRS(i));
    }

    // Track and/or hit information filled as per config
    htstats->Fill(6);
    hmeans->Fill(m_dedx);
    if (eBhabha) tBhabha->Fill();
    if (eRadBhabha) tRadee->Fill();
  }
}
