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

REG_MODULE(CDCDedxDQM)

//---------------------------------
CDCDedxDQMModule::CDCDedxDQMModule(): HistoModule()
{
  setPropertyFlags(c_ParallelProcessingCertified); // parallel processing
  setDescription("CDC dE/dx DQM plots with bhabha/hadron events.");
}

//---------------------------------
CDCDedxDQMModule::~CDCDedxDQMModule()
{

}

//---------------------------------
void CDCDedxDQMModule::defineHisto()
{

  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("CDCDedx");
  oldDir->cd("CDCDedx");

  if (m_MetaDataPtr) {
    m_exp = int(m_MetaDataPtr->getExperiment());
    m_run = int(m_MetaDataPtr->getRun());
    if (m_DBRunGain)m_rungain = m_DBRunGain->getRunGain();
  }

  hMeta = new TH1D("hMeta", "hMeta", 3, 0.5, 3.5);
  hMeta->GetXaxis()->SetTitle("Quantity");
  hMeta->GetYaxis()->SetTitle("Values");
  hMeta->SetTitle(Form("(Exp:%d, Run:%d, RG:%0.03f)", m_exp, m_run, m_rungain));
  hMeta->GetXaxis()->SetBinLabel(1, "nevt");
  hMeta->GetXaxis()->SetBinLabel(2, "nbhabha");
  hMeta->GetXaxis()->SetBinLabel(3, "nhadron");

  hdEdx = new TH1D("hdEdx", ";CDC dE/dx;Entries", 300, 0., 2.5);
  hdEdxvsP = new TH2D("hdEdxVsP", ";#it{p}_{CDC} (GeV/c);CDC dE/dx", 1600, 0.050, 4.50, 2000, 0.35, 20.2);
  hdEdxvsEvt = new TH2D("hdEdxvsEvt", ";Events(M);CDC dE/dx", 300, 0, 300, 500, 0.00, 2.5);
  hdEdxvsPhi = new TH2D("hdEdxvsPhi", ";#phi (e^{-}e^{+} tracks);CDC dE/dx", 320, -3.20, 3.20, 500, 0.00, 2.5);
  hdEdxvsCosth = new TH2D("hdEdxvsCosth", ";cos#theta (e^{-}e^{+} tracks);CDC dE/dx", 200, -1.00, 1.00, 500, 0.00, 2.5);
  hWires = new TH2F("hWires", "All Wires;", 2400, -1.2, 1.2, 2400, -1.2, 1.2);
  hWires->GetXaxis()->SetTitle("CDC-wire map: counter-clockwise and start from +x");
  hWireStatus = new TH2F("hWireStatus", "Wire Status", 2400, -1.2, 1.2, 2400, -1.2, 1.2);
  hWireStatus->GetXaxis()->SetTitle("CDC-wire map: counter-clockwise and start from +x");

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
  hdEdxvsP->Reset();
  hdEdxvsPhi->Reset();
  hdEdxvsCosth->Reset();
  hdEdxvsEvt->Reset();
  hWires->Reset();
  hWireStatus->Reset();

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
  if (m_MetaDataPtr)m_event = int(m_MetaDataPtr->getEvent());

  for (Int_t idedx = 0; idedx < m_cdcDedxTracks.getEntries(); idedx++) {

    CDCDedxTrack* dedxTrack = m_cdcDedxTracks[idedx];
    if (!dedxTrack || dedxTrack->size() == 0)continue;

    const Track* track = dedxTrack->getRelatedFrom<Track>();
    if (!track)continue;

    const TrackFitResult* fitResult = track->getTrackFitResultWithClosestMass(Const::pion);
    if (!fitResult)continue;

    if (fabs(fitResult->getD0()) >= 2.0)continue;
    if (fabs(fitResult->getZ0()) >= 2.0)continue;

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

    double pTrk = fitResult->getMomentum().Mag();
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
      hdEdxvsPhi->Fill(phi, dedxnosat);
      hdEdxvsCosth->Fill(costh, dedxnosat);

      if (m_event > 150e6)m_event = 150e6 - 100;
      m_event = int(m_event / 5e5);
      hdEdxvsEvt->Fill(m_event, dedxnosat);

    }

    if (IsHadronEvt)hdEdxvsP->Fill(pCDC, dedx);

    for (int ihit = 0; ihit < dedxTrack->size(); ++ihit) {
      int iwire = dedxTrack->getWire(ihit);
      double iadc = dedxTrack->getADCCount(ihit);
      if (m_adc[iwire].size() < 50)m_adc[iwire].push_back(iadc); //just contiung dead
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

  //get dead wire pattern
  Int_t nbadwires = 0;
  for (int iwire = 0; iwire < 14336; ++iwire) {
    int nwire = getIndexVal(iwire, "nwirelayer");
    int twire = getIndexVal(iwire, "twire");
    double radius = getIndexVal(iwire, "rwire");
    int wire = iwire - twire ;
    double phi = 2.*TMath::Pi() * (float(wire) / float(nwire));
    double x = radius * cos(phi);
    double y = radius * sin(phi);
    hWires->Fill(x, y);
    if (m_adc[iwire].size() > 0)continue;
    nbadwires++;
    hWireStatus->Fill(x, y);
  }
  hWireStatus->SetTitle(Form("%d", nbadwires));

}


//---------------------------------
void CDCDedxDQMModule::terminate()
{

}

//-----------------------------------------------------------
double CDCDedxDQMModule::getIndexVal(int iWire, TString what)
{
  //few hardcoded number
  //radius of each CDC layer
  double r[56] = {
    16.80,  17.80,  18.80,  19.80,  20.80,  21.80,  22.80,  23.80,
    25.70,  27.52,  29.34,  31.16,  32.98,  34.80,
    36.52,  38.34,  40.16,  41.98,  43.80,  45.57,
    47.69,  49.46,  51.28,  53.10,  54.92,  56.69,
    58.41,  60.18,  62.00,  63.82,  65.64,  67.41,
    69.53,  71.30,  73.12,  74.94,  76.76,  78.53,
    80.25,  82.02,  83.84,  85.66,  87.48,  89.25,
    91.37,  93.14,  94.96,  96.78,  98.60, 100.37,
    102.09, 103.86, 105.68, 107.50, 109.32, 111.14
  };

  Int_t totalWireiLayer = 0 ;
  double myreturn = 0;
  for (Int_t iLayer = 0; iLayer < 56; iLayer++) {
    int iSuperLayer = (iLayer - 2) / 6;
    if (iSuperLayer <= 0)iSuperLayer = 1;
    int nWireiLayer = 160 + (iSuperLayer - 1) * 32;
    totalWireiLayer += nWireiLayer;

    if (iWire < totalWireiLayer) {
      if (what == "layer")myreturn = iLayer;
      else if (what == "nwirelayer") myreturn = nWireiLayer;
      else if (what == "twire")  myreturn = totalWireiLayer - nWireiLayer;
      else if (what == "rwire")  myreturn = r[iLayer] / 100.;
      else std::cout << "Invalid return :0 " << std::endl;
      break;
    }
  }
  return myreturn;
}
