/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxDQM/CDCDedxDQM.h>

#include <TDirectory.h>

using namespace Belle2;

REG_MODULE(CDCDedxDQM);

//---------------------------------
CDCDedxDQMModule::CDCDedxDQMModule(): HistoModule()
{
  setPropertyFlags(c_ParallelProcessingCertified); // parallel processing
  setDescription("CDC dE/dx DQM plots with bhabha/hadron events.");
  addParam("mmode", mmode, "default monitoring mode is basic", std::string("basic"));
}

//---------------------------------
void CDCDedxDQMModule::defineHisto()
{

  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("CDCDedx");
  oldDir->cd("CDCDedx");

  int expNum = -1;
  int runNum = -1;
  double rungain = -99.0;

  if (m_MetaDataPtr) {
    expNum = int(m_MetaDataPtr->getExperiment());
    runNum = int(m_MetaDataPtr->getRun());
    if (m_DBRunGain) rungain = m_DBRunGain->getRunGain();
  }

  hMeta = new TH1D("hMeta", "hMeta", 3, 0.5, 3.5);
  hMeta->GetXaxis()->SetTitle("Quantity");
  hMeta->GetYaxis()->SetTitle("Values");
  hMeta->SetTitle(Form("(Exp:%d, Run:%d, RG:%0.03f)", expNum, runNum, rungain));
  hMeta->GetXaxis()->SetBinLabel(1, "nevt");
  hMeta->GetXaxis()->SetBinLabel(2, "nbhabha");
  hMeta->GetXaxis()->SetBinLabel(3, "nhadron");

  hdEdx = new TH1D("hdEdx", ";CDC dE/dx;Entries", 250, 0., 2.5);
  hinjtimeHer = new TH2D("hinjtimeHer", ";injection time (#mu s); CDC dE/dx", 40, 0, 80e3, 50, 0, 2.5);
  hinjtimeLer = new TH2D("hinjtimeLer", ";injection time (#mu s); CDC dE/dx", 40, 0, 80e3, 50, 0, 2.5);
  hdEdxvsP = new TH2D("hdEdxVsP", ";#it{p}_{CDC} (GeV/c);CDC dE/dx", 400, 0.050, 4.50, 800, 0.35, 20.35);
  hdEdxvsEvt = new TH2D("hdEdxvsEvt", ";Events(M);CDC dE/dx", 300, 0, 300, 200, 0.00, 2.5);
  hdEdxvsCosth = new TH2D("hdEdxvsCosth", ";cos#theta (e^{-}e^{+} tracks);CDC dE/dx", 100, -1.00, 1.00, 250, 0.00, 2.5);
  hdEdxvsPhi = new TH2D("hdEdxvsPhi", ";#phi (e^{-}e^{+} tracks);CDC dE/dx", 100, -3.20, 3.20, 250, 0.00, 2.5);
  if (mmode != "basic") {
    hWires = new TH2F("hWires", "All Wires;", 2400, -1.2, 1.2, 2400, -1.2, 1.2);
    hWires->GetXaxis()->SetTitle("CDC-wire map: counter-clockwise and start from +x");
    hWireStatus = new TH2F("hWireStatus", "Wire Status", 2400, -1.2, 1.2, 2400, -1.2, 1.2);
    hWireStatus->GetXaxis()->SetTitle("CDC-wire map: counter-clockwise and start from +x");
  }
  oldDir->cd();

}


//---------------------------------
void CDCDedxDQMModule::initialize()
{

  if (!m_cdcDedxTracks.isOptional()) {
    B2WARNING("Missing CDCDedxTracks array, CDCDedxDQM is skipped.");
    return;
  }

  m_TrgResult.isOptional();
  m_cdcDedxTracks.isRequired();
  REG_HISTOGRAM

}

//-------------------------------
void CDCDedxDQMModule::beginRun()
{

  if (!m_cdcDedxTracks.isOptional()) {
    B2WARNING("Missing CDCDedxTracks array, CDCDedxDQM is skipped.");
    return;
  }

  hMeta->Reset();
  hdEdx->Reset();
  hinjtimeHer->Reset();
  hinjtimeLer->Reset();
  hdEdxvsP->Reset();
  hdEdxvsCosth->Reset();
  hdEdxvsPhi->Reset();
  hdEdxvsEvt->Reset();
  if (mmode != "basic") {
    hWires->Reset();
    hWireStatus->Reset();
  }
}


//----------------------------
void CDCDedxDQMModule::event()
{

  if (!m_cdcDedxTracks.isOptional())  return;

  if (!m_TrgResult.isValid()) {
    B2WARNING("Required SoftwareTriggerResult object not available: CDCDedxDQM is skipped");
    return;
  }

  const std::map<std::string, int>& fresults = m_TrgResult->getResults();
  if (fresults.find("software_trigger_cut&skim&accept_bhabha") == fresults.end()
      and fresults.find("software_trigger_cut&skim&accept_hadron") == fresults.end())return;

  const bool IsBhabhaEvt = (m_TrgResult->getResult("software_trigger_cut&skim&accept_bhabha") ==
                            SoftwareTriggerCutResult::c_accept);
  const bool IsHadronEvt = (m_TrgResult->getResult("software_trigger_cut&skim&accept_hadron") ==
                            SoftwareTriggerCutResult::c_accept);

  m_nEvt += 1;
  if (!IsBhabhaEvt and !IsHadronEvt)return;
  if (IsBhabhaEvt)m_nBEvt += 1;
  if (IsHadronEvt)m_nHEvt += 1;

  //Get current evt number
  int event = -1; /**< number of event */

  if (m_MetaDataPtr)event = int(m_MetaDataPtr->getEvent());

  for (int idedx = 0; idedx < m_cdcDedxTracks.getEntries(); idedx++) {

    CDCDedxTrack* dedxTrack = m_cdcDedxTracks[idedx];
    if (!dedxTrack || dedxTrack->size() == 0)continue;

    const Track* track = dedxTrack->getRelatedFrom<Track>();
    if (!track)continue;

    const TrackFitResult* fitResult = track->getTrackFitResultWithClosestMass(Const::pion);
    if (!fitResult)continue;

    UncertainHelix helix = fitResult->getUncertainHelix();
    static DBObjPtr<BeamSpot> beamSpotDB;
    helix.passiveMoveBy(ROOT::Math::XYZVector(beamSpotDB->getIPPosition()));
    const auto& frame = ReferenceFrame::GetCurrent();
    double dr = frame.getVertex(ROOT::Math::XYZVector(helix.getPerigee())).Rho();
    double dz = frame.getVertex(ROOT::Math::XYZVector(helix.getPerigee())).Z();
    if (dr >= 1.0 || fabs(dz) >= 1.0)continue;

    //CDC acceptance as well
    double costh = dedxTrack->getCosTheta();
    if (costh < TMath::Cos(150.0 * TMath::DegToRad()))continue;
    if (costh > TMath::Cos(17.0 * TMath::DegToRad())) continue;

    //clean tracks with relaxed nhit cut
    double nhits = dedxTrack->getNLayerHits();
    if (costh > -0.55 && costh < 0.820) {
      if (nhits < 20)continue;
    } else {
      if (costh <= -0.62 || costh >= 0.880) {
        if (nhits < 8)continue;
        if (costh > 0 && nhits < 10)continue;
      } else {
        if (nhits < 15)continue;
      }
    }

    double dedxnosat = dedxTrack->getDedxNoSat();
    if (dedxnosat < 0)continue;

    double dedx = dedxTrack->getDedx();
    if (dedx < 0)continue;

    double pCDC = dedxTrack->getMomentum();
    if (pCDC <= 0) continue;

    double pTrk = fitResult->getMomentum().R();
    if (pTrk <= 0) continue;

    if (IsBhabhaEvt) {
      const ECLCluster* eclCluster = track->getRelated<ECLCluster>();
      if (eclCluster and eclCluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
        double TrkEoverP = eclCluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons) / pTrk;
        if (TrkEoverP > 0) {
          if (abs(TrkEoverP - 1.0) > 0.25)continue;
        }
      }

      hdEdx->Fill(dedxnosat);
      double phi = fitResult->getMomentum().Phi();
      if (hdEdxvsCosth->Integral() <= 80000)hdEdxvsCosth->Fill(costh, dedxnosat);
      if (hdEdxvsPhi->Integral() <= 80000)hdEdxvsPhi->Fill(phi, dedxnosat);

      if (event >= 150e6)event = 150e6 - 100;
      event = int(event / 5e5);
      hdEdxvsEvt->Fill(event, dedxnosat);

      // And check if the stored data is valid and if an injection happened recently
      if (TTDInfo->isValid() && TTDInfo->hasInjection()) {
        if (TTDInfo->isHER())
          hinjtimeHer->Fill(TTDInfo->getTimeSinceLastInjectionInMicroSeconds(), dedxnosat);
        else
          hinjtimeLer->Fill(TTDInfo->getTimeSinceLastInjectionInMicroSeconds(), dedxnosat);
      } else
        return;

    }
    if (IsHadronEvt && hdEdxvsP->Integral() <= 80000)hdEdxvsP->Fill(pCDC, dedx);

    if (mmode != "basic") {
      for (int ihit = 0; ihit < dedxTrack->size(); ++ihit) {
        int iwire = dedxTrack->getWire(ihit);
        double iadc = dedxTrack->getADCCount(ihit);
        if (m_adc[iwire].size() < 50)m_adc[iwire].push_back(iadc); //just contiung dead
      }
    }
  }

}

//---------------------------------
void CDCDedxDQMModule::endRun()
{

  hMeta->SetBinContent(1, m_nEvt);
  hMeta->SetBinContent(2, m_nBEvt);
  hMeta->SetBinContent(3, m_nHEvt);

  if (hdEdx->GetEntries() > 0) {
    hdEdx->GetXaxis()->SetRange(hdEdx->FindFirstBinAbove(0, 1), hdEdx->FindLastBinAbove(0, 1));
  }

  if (hdEdxvsEvt->GetEntries() > 0) {
    hdEdxvsEvt->GetXaxis()->SetRange(hdEdxvsEvt->FindFirstBinAbove(0, 1), hdEdxvsEvt->FindLastBinAbove(0, 1));
  }

  if (hinjtimeHer->GetEntries() > 0) {
    hinjtimeHer->GetXaxis()->SetRange(hinjtimeHer->FindFirstBinAbove(0, 1), hinjtimeHer->FindLastBinAbove(0, 1));
  }
  if (hinjtimeLer->GetEntries() > 0) {
    hinjtimeLer->GetXaxis()->SetRange(hinjtimeLer->FindFirstBinAbove(0, 1), hinjtimeLer->FindLastBinAbove(0, 1));
  }
  //get dead wire pattern
  if (mmode != "basic") plotWireMap();
}


//---------------------------------
void CDCDedxDQMModule::terminate()
{
  B2INFO("CDCDedxDQMModule: terminate called");
}

//------------------------------------
void CDCDedxDQMModule::plotWireMap()
{

  B2INFO("Creating CDCGeometryPar object");
  Belle2::CDC::CDCGeometryPar& cdcgeo = Belle2::CDC::CDCGeometryPar::Instance();

  int jwire = -1;
  int nbadwires = 0;

  for (unsigned int ilay = 0; ilay < c_maxNSenseLayers; ++ilay) {
    for (unsigned int iwire = 0; iwire < cdcgeo.nWiresInLayer(ilay); ++iwire) {
      jwire++;
      double phi = 2.*TMath::Pi() * (iwire / double(cdcgeo.nWiresInLayer(ilay)));
      double radius = cdcgeo.senseWireR(ilay) / 100.;
      double x = radius * cos(phi);
      double y = radius * sin(phi);
      hWires->Fill(x, y);
      if (m_adc[jwire].size() > 0)continue;
      nbadwires++;
      hWireStatus->Fill(x, y);
    }
  }
  hWireStatus->SetTitle(Form("%d", nbadwires));
}
