/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxElectronCollector/CDCDedxElectronCollectorModule.h>

#include <TTree.h>
#include <TH1D.h>
#include <TH1I.h>
#include <TMath.h>

using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCDedxElectronCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxElectronCollectorModule::CDCDedxElectronCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("A collector module for CDC dE/dx electron calibrations");

  // Parameter definitions
  addParam("cleanupCuts", m_cuts, "boolean to apply cleanup cuts", true);
  addParam("m_maxHits", m_maxHits, "maximum number of hits per track ", int(100));
  addParam("m_setEoP", m_setEoP, "cluster eop window around 1.0 ", double(0.25));
  addParam("isCosth", isCosth, "true for adding costh tree branch. ", false);
  addParam("isMom", isMom, "true for adding momentum tree branch. ", false);
  addParam("isCharge", isCharge, "true for charge dedx tree branch. ", false);
  addParam("isRun", isRun, "true for adding run number tree branch. ", false);
  addParam("isWire", isWire, "true for adding wires tree branch. ", false);
  addParam("isLayer", isLayer, "true for adding layers tree branch. ", false);
  addParam("isDoca", isDoca, "true for adding doca tree branch. ", false);
  addParam("isEnta", isEnta, "true for adding enta tree branch. ", false);
  addParam("isInjTime", isInjTime, "true for adding time var tree branch. ", false);
  addParam("isDocaRS", isDocaRS, "true for adding doca tree branch. ", false);
  addParam("isEntaRS", isEntaRS, "true for adding enta tree branch. ", false);
  addParam("isDedxHit", isDedxHit, "true for adding dedxhit tree branch. ", false);
  addParam("isBhabha", isBhabha, "true for bhabha events", true);
  addParam("isRadee", isRadee, "true for radee events", false);
  addParam("isTrgSel", isTrgSel, "true to enable trigger sel inside module", false);
}

//-----------------------------------------------------------------
//                 Create ROOT objects
//-----------------------------------------------------------------

void CDCDedxElectronCollectorModule::prepare()
{
  m_trgResult.isOptional();
  m_dedxTracks.isRequired();
  m_tracks.isRequired();
  m_trackFitResults.isRequired();

  // Data object creation
  auto means = new TH1D("means", "CDC dE/dx truncated means", 250, 0, 2.5);
  auto ttree = new TTree("tree", "Tree with dE/dx information");

  auto hestats = new TH1I("hestats", "Event Stats", 6, -0.5, 5.5);
  hestats->SetFillColor(kYellow);
  hestats->GetXaxis()->SetBinLabel(1, "all");
  hestats->GetXaxis()->SetBinLabel(2, "notrig");
  hestats->GetXaxis()->SetBinLabel(3, "noskim");
  hestats->GetXaxis()->SetBinLabel(4, "wrongskim");
  hestats->GetXaxis()->SetBinLabel(5, "unclean");
  hestats->GetXaxis()->SetBinLabel(6, "selected");

  auto htstats = new TH1I("htstats", "track Stats", 7, -0.5, 6.5);
  htstats->SetFillColor(kYellow);
  htstats->GetXaxis()->SetBinLabel(1, "alltrk");
  htstats->GetXaxis()->SetBinLabel(2, "vtx");
  htstats->GetXaxis()->SetBinLabel(3, "inCDC");
  htstats->GetXaxis()->SetBinLabel(4, "whits");
  htstats->GetXaxis()->SetBinLabel(5, "weop");
  htstats->GetXaxis()->SetBinLabel(6, "radee");
  htstats->GetXaxis()->SetBinLabel(7, "selected");

  if (isInjTime) {
    ttree->Branch<double>("injtime", &m_injTime);
    ttree->Branch<double>("injring", &m_injRing);
  }

  ttree->Branch<double>("dedx", &m_dedx);
  if (isCosth)ttree->Branch<double>("costh", &m_costh);
  if (isMom)ttree->Branch<double>("p", &m_p);
  if (isCharge)ttree->Branch<int>("charge", &m_charge);
  if (isRun)ttree->Branch<int>("run", &m_run);

  if (isWire)ttree->Branch("wire", &m_wire);
  if (isLayer)ttree->Branch("layer", &m_layer);
  if (isDoca)ttree->Branch("doca", &m_doca);
  if (isEnta)ttree->Branch("enta", &m_enta);
  if (isDocaRS)ttree->Branch("docaRS", &m_docaRS);
  if (isEntaRS)ttree->Branch("entaRS", &m_entaRS);
  if (isDedxHit)ttree->Branch("dedxhit", &m_dedxHit);

  // Collector object registration
  registerObject<TH1D>("means", means);
  registerObject<TTree>("tree", ttree);
  registerObject<TH1I>("hestats", hestats);
  registerObject<TH1I>("htstats", htstats);
}

//-----------------------------------------------------------------
//                 Fill ROOT objects
//-----------------------------------------------------------------
void CDCDedxElectronCollectorModule::collect()
{

  auto hestats = getObjectPtr<TH1I>("hestats");
  hestats->Fill(0);

  if (isTrgSel) {
    if (!m_trgResult.isValid()) {
      B2WARNING("SoftwareTriggerResult required to select bhabha/radee event is not found");
      hestats->Fill(1);
      return;
    }

    //release05: bhabha_all is grand skim = bhabha+bhabhaecl+radee
    const std::map<std::string, int>& fresults = m_trgResult->getResults();
    if (fresults.find("software_trigger_cut&skim&accept_bhabha") == fresults.end() and
        fresults.find("software_trigger_cut&skim&accept_radee") == fresults.end()) {
      B2WARNING("Can't find required bhabha/radee trigger identifiers");
      hestats->Fill(2);
      return;
    }

    const bool eBhabha = (m_trgResult->getResult("software_trigger_cut&skim&accept_bhabha") ==
                          SoftwareTriggerCutResult::c_accept);

    const bool eRadBhabha = (m_trgResult->getResult("software_trigger_cut&skim&accept_radee") ==
                             SoftwareTriggerCutResult::c_accept);

    if (!isBhabha && !isRadee) {
      B2WARNING("requested not-supported event type: going back");
      hestats->Fill(3);
      return;
    } else if (isBhabha && !isRadee && !eBhabha) {
      B2WARNING("requested bhabha only but event not found: going back");
      hestats->Fill(3);
      return;
    } else  if (isRadee && !isBhabha && !eRadBhabha) {
      B2WARNING("requested radee only but event not found: going back");
      hestats->Fill(3);
      return;
    }
  } else {
    hestats->GetXaxis()->SetBinLabel(2, "inact1");
    hestats->GetXaxis()->SetBinLabel(3, "inact2");
    hestats->GetXaxis()->SetBinLabel(4, "inact3");
  }

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  int run = eventMetaDataPtr->getRun();
  if (isRun)m_run = run;

  int nTracks = m_dedxTracks.getEntries();
  if (nTracks >= 4) {
    B2WARNING("too many tracks: unclean bhabha or radee event: " << nTracks);
    hestats->Fill(4);
    return;
  }

  hestats->Fill(5);

  //Collector object access
  auto tree = getObjectPtr<TTree>("tree");
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
    m_injTime = dedxTrack->getInjectionTime();
    m_injRing = dedxTrack->getInjectionRing();
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

      m_nhits = dedxTrack->size();
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
    if (isRadee) {
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
    if (isWire)m_wire.clear();
    if (isLayer)m_layer.clear();
    if (isDoca)m_doca.clear();
    if (isEnta)m_enta.clear();
    if (isDocaRS)m_docaRS.clear();
    if (isEntaRS)m_entaRS.clear();
    if (isDedxHit)m_dedxHit.clear();

    // Simple numbers don't need to be cleared
    // make sure to use the truncated mean without the hadron saturation correction

    for (int i = 0; i < m_nhits; ++i) {
      // if (m_DBWireGains->getWireGain(dedxTrack->getWire(i)) == 0)continue;
      if (isWire)m_wire.push_back(dedxTrack->getWire(i));
      if (isLayer)m_layer.push_back(dedxTrack->getHitLayer(i));
      if (isDoca)m_doca.push_back(dedxTrack->getDoca(i));
      if (isEnta)m_enta.push_back(dedxTrack->getEnta(i));
      if (isDocaRS)m_docaRS.push_back(dedxTrack->getDocaRS(i) / dedxTrack->getCellHalfWidth(i));
      if (isEntaRS)m_entaRS.push_back(dedxTrack->getEntaRS(i));
      if (isDedxHit)m_dedxHit.push_back(dedxTrack->getDedx(i));
    }

    // Track and/or hit information filled as per config
    htstats->Fill(6);
    hmeans->Fill(m_dedx);
    tree->Fill();
  }
}
