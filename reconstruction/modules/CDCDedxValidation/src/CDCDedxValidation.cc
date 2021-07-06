/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxValidation/CDCDedxValidation.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>

#include <TH2D.h>

using namespace Belle2;


REG_MODULE(CDCDedxValidation)

//-------------------------------------------------
CDCDedxValidationModule::CDCDedxValidationModule():
  HistoModule(),
  fD0Window(1.0),
  fZ0Window(1.0),
  fnRunCounter(0),
  fiRun(0),
  fnBinsdedx(120),
  fnBinsdedxLE(0.4),
  fnBinsdedxUE(1.6)
{
  setDescription("Make data quality monitoring plots for CDC dE/dx");
  addParam("outputFileName", fOutFileName, "Name for output file", std::string("CDCdEdxValidation.root"));
  addParam("SampleType", fCollType, "Switch to hadron (false) vs bhabha files", std::string("temp"));
  addParam("fnRuns", fnRuns, "Number of input runs");

}

//-----------------------------------------
void CDCDedxValidationModule::initialize()
{

  m_cdcDedxTracks.isRequired();

  if (fCollType != "radbhabha" and fCollType != "bhabha" and fCollType != "hadron") {
    printf("Wrong input file tpye (%s): choose bhabha or radbhabha or hadron \n", fCollType.data());
    return;
  } else {
    fBasic = new TList(); fBasic->SetOwner(); fBasic->SetName("AllRunBasics");
    fPRdEdx = new TList(); fPRdEdx->SetOwner(); fPRdEdx->SetName("PerRunDEdx");
    fPRdEdxinP = new TList(); fPRdEdxinP->SetOwner(); fPRdEdxinP->SetName("PerRundEdxinP");
    DefineHistograms("AR", 0);
  }
}


//---------------------------------------
void CDCDedxValidationModule::beginRun()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  fCurrentRunNum = eventMetaDataPtr->getRun();

  fiRun = fnRunCounter;
  fcRunGain = -1.0;

  DefineHistograms("PR", fiRun);

  fnRunCounter++;
}


//------------------------------------
void CDCDedxValidationModule::event()
{

  //Loop over CDC Tracks
  for (Int_t idedx = 0; idedx < m_cdcDedxTracks.getEntries(); idedx++) {

    CDCDedxTrack* dedxTrack = m_cdcDedxTracks[idedx];
    if (!dedxTrack) {
      ((TH1D*)fBasic->FindObject("hTrkPerEvtStats"))->Fill(0.0);
      continue;
    }

    const Track* track = dedxTrack->getRelatedFrom<Track>();
    if (!track) {
      ((TH1D*)fBasic->FindObject("hTrkPerEvtStats"))->Fill(1.0);
      continue;
    }

    const TrackFitResult* mTrack = NULL;
    if (fCollType == "bhabha" || fCollType == "radbhabha") {
      mTrack = track->getTrackFitResultWithClosestMass(Const::electron);
    } else {
      mTrack = track->getTrackFitResultWithClosestMass(Const::pion);
    }
    if (!mTrack) {
      ((TH1D*)fBasic->FindObject("hTrkPerEvtStats"))->Fill(2.0);
      continue;
    }

    bool IsTrkSelected = IsSelectedTrack(mTrack);
    if (!IsTrkSelected) {
      ((TH1D*)fBasic->FindObject("hTrkPerEvtStats"))->Fill(3.0);
      continue;
    }

    if (dedxTrack->getNLayerHits() <= 20) continue;

    const ECLCluster* eclCluster = track->getRelated<ECLCluster>();
    if (eclCluster and eclCluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
      fTrkEoverP = (eclCluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons)) / (mTrack->getMomentum().Mag());
      if (fCollType == "bhabha" || fCollType == "radbhabha") {
        if (abs(fTrkEoverP - 1.0) >= 0.2)continue;
        ((TH1D*)fBasic->FindObject(Form("hEOverP_AR")))->Fill(double(fTrkEoverP));
      }
    } else {
      ((TH1D*)fBasic->FindObject("hTrkPerEvtStats"))->Fill(4.0);
      continue;

    }

    ((TH1D*)fBasic->FindObject("hTrkPerEvtStats"))->Fill(5.0);
    FillHistograms(dedxTrack, mTrack);
  }
}

//----------------------------------------------------------------------------------------------------
void CDCDedxValidationModule::FillHistograms(CDCDedxTrack* dedxTrack, const TrackFitResult* mTrack)
{

  fcRunGain = dedxTrack->getRunGain();

  Int_t TrkCharge = mTrack->getChargeSign();
  Double_t TrkdEdxnosat = dedxTrack->getDedxNoSat();
  Double_t TrkdEdx = dedxTrack->getDedx();
  Double_t TrkCosTheta = dedxTrack->getCosTheta();
  Double_t TrkMom = dedxTrack->getMomentum();

  if (TrkMom >= 8.00)TrkMom = 7.999;
  Double_t BWMom = 0.250; //in GeV
  Int_t iMomBin = Int_t(TrkMom / BWMom);

  if (fCollType == "radbhabha" or fCollType == "bhabha") {

    if (TrkCharge > 0) {
      ((TH1D*)fBasic->FindObject(Form("hP_Positron_AR")))->Fill(TrkMom);
      ((TH1D*)fBasic->FindObject(Form("hdEdx_Positron_AR")))->Fill(TrkdEdxnosat);
      ((TH1D*)fPRdEdxinP->FindObject(Form("hdEdx_Posi_Pbin_AR%d", iMomBin)))->Fill(TrkdEdxnosat);
    } else if (TrkCharge < 0) {
      ((TH1D*)fBasic->FindObject(Form("hP_Electron_AR")))->Fill(TrkMom);
      ((TH1D*)fBasic->FindObject(Form("hdEdx_Electron_AR")))->Fill(TrkdEdxnosat);
      ((TH1D*)fPRdEdxinP->FindObject(Form("hdEdx_Elec_Pbin_AR%d", iMomBin)))->Fill(TrkdEdxnosat);
    }

    ((TH1D*)fBasic->FindObject(Form("hdEdx_AR")))->Fill(double(TrkdEdxnosat));
    ((TH2D*)fBasic->FindObject(Form("hdEdxvsPhi_AR")))->Fill(double(mTrack->getPhi()), double(TrkdEdxnosat));
    ((TH2D*)fBasic->FindObject(Form("hPvsdEdx_AR")))->Fill(TrkMom * TrkCharge, double(TrkdEdxnosat));
    ((TH2D*)fBasic->FindObject(Form("hPvsCosth_AR")))->Fill(TrkMom * TrkCharge, double(TrkCosTheta));

    hdEdx_PR[fiRun]->Fill(double(TrkdEdxnosat));

  } else if (fCollType == "hadron") {

    // double ChiE = dedxTrack->getChi(0);
    // double ChiMu = dedxTrack->getChi(1);
    double ChiPi = dedxTrack->getChi(2);
    double ChiK = dedxTrack->getChi(3);
    double ChiP = dedxTrack->getChi(4);
    // double ChiD = dedxTrack->getChi(5);

    ((TH2D*)fBasic->FindObject(Form("hPvsdEdx_hadAR")))->Fill(TrkMom, TrkdEdx);

    if ((TrkMom < 0.40) && (fTrkEoverP < 0.4) && (TrkdEdx < (0.6 + 0.10 / (TrkMom * TrkMom)))
        && (TrkdEdx > (0.4 + 0.012 / (TrkMom * TrkMom)))) {
      ((TH1D*)fBasic->FindObject(Form("hPionChiallP")))->Fill(ChiPi);
      if (TrkMom < 0.300)((TH1D*)fBasic->FindObject(Form("hPionChiLowP")))->Fill(ChiPi);
      else ((TH1D*)fBasic->FindObject(Form("hPionChiHighP")))->Fill(ChiPi);
      ((TH2D*)fBasic->FindObject(Form("hPvsdEdxPion_hadAR")))->Fill(TrkMom, TrkdEdx);
    }

    if ((TrkMom < 0.40) && (TrkdEdx > 1.35) && (TrkdEdx < (0.6 + 0.40 / (TrkMom * TrkMom)))
        && (TrkdEdx > (0.6 + 0.10 / (TrkMom * TrkMom)))) {
      ((TH1D*)fBasic->FindObject(Form("hKaonChiallP")))->Fill(ChiK);
      if (TrkMom < 0.350)((TH1D*)fBasic->FindObject(Form("hKaonChiLowP")))->Fill(ChiK);
      else ((TH1D*)fBasic->FindObject(Form("hKaonChiHighP")))->Fill(ChiK);
      ((TH2D*)fBasic->FindObject(Form("hPvsdEdxKaon_hadAR")))->Fill(TrkMom, TrkdEdx);
    }

    if ((TrkMom < 0.80) && (TrkdEdx > 1.35) && (TrkdEdx < (0.6 + 1.20 / (TrkMom * TrkMom)))
        && (TrkdEdx > (0.6 + 0.40 / (TrkMom * TrkMom)))) {
      ((TH1D*)fBasic->FindObject(Form("hProtonChiallP")))->Fill(ChiP);
      if (TrkMom < 0.600)((TH1D*)fBasic->FindObject(Form("hProtonChiLowP")))->Fill(ChiP);
      else ((TH1D*)fBasic->FindObject(Form("hProtonChiHighP")))->Fill(ChiP);
      ((TH2D*)fBasic->FindObject(Form("hPvsdEdxProton_hadAR")))->Fill(TrkMom, TrkdEdx);
    }
  }

}



//----------------------------------
void CDCDedxValidationModule::endRun()
{
  if (fCollType != "hadron")ExtractHistograms("PR");
}


//----------------------------------------------------------------------------------
void CDCDedxValidationModule::DefineHistograms(TString level = "XR", Int_t iR = 123)
{
  if (level == "AR") {

    TH1D* hTrkPerEvtStats = new TH1D("hTrkPerEvtStats", "Track selections", 6, -0.5, 5.5);
    hTrkPerEvtStats->GetXaxis()->SetBinLabel(1, "no dedxobject");
    hTrkPerEvtStats->GetXaxis()->SetBinLabel(2, "no assoc-track");
    hTrkPerEvtStats->GetXaxis()->SetBinLabel(3, "no TrackFit");
    hTrkPerEvtStats->GetXaxis()->SetBinLabel(4, "no pass cuts");
    hTrkPerEvtStats->GetXaxis()->SetBinLabel(5, "no eclCluster");
    hTrkPerEvtStats->GetXaxis()->SetBinLabel(6, "Selected");
    hTrkPerEvtStats->SetFillColor(kRed);
    hTrkPerEvtStats->SetFillStyle(3015);
    hTrkPerEvtStats->SetMinimum(0);
    fBasic->Add(hTrkPerEvtStats);

    if (fCollType == "radbhabha" or fCollType == "bhabha") {

      TH1D* hdEdx_AR = new TH1D("hdEdx_AR", "dE/dx (nohad sat)", fnBinsdedx, fnBinsdedxLE, fnBinsdedxUE);
      hdEdx_AR->GetXaxis()->SetTitle(Form("dE/dx truncMean of %s tracks", fCollType.data()));
      hdEdx_AR->GetYaxis()->SetTitle("Entries");
      fBasic->Add(hdEdx_AR);

      TH1D* hEOverP_AR = new TH1D("hEOverP_AR", "E/p distribution", 100, 0.5, 1.5);
      hEOverP_AR->GetXaxis()->SetTitle("E/p distribution");
      hEOverP_AR->GetYaxis()->SetTitle("Entries");
      fBasic->Add(hEOverP_AR);

      TH1D* hRunGainPR = new TH1D("hRunGainPR", "bla-bla", fnRuns, -0.5, fnRuns - 0.5);
      hRunGainPR->SetTitle("Run gain variation vs. RunNumber;Run Numbers;dE/dx mean");
      hRunGainPR->GetYaxis()->SetRangeUser(0.85, 1.15);
      fBasic->Add(hRunGainPR);

      TH1D* hP_Electron_AR = new TH1D("hP_Electron_AR", "bla-bla", 320, 0.0, 8.0);
      hP_Electron_AR->SetTitle("Momentum distribution of e-; Momentum of (e-); Entries");
      fBasic->Add(hP_Electron_AR);

      TH1D* hP_Positron_AR = new TH1D("hP_Positron_AR", "bla-bla", 320, 0.0, 8.0);
      hP_Positron_AR->SetTitle("Momentum distribution of e+;Momentum of (e+);Entries");
      fBasic->Add(hP_Positron_AR);

      TH1D* hdEdx_Electron_AR = new TH1D("hdEdx_Electron_AR", "bla-bla", fnBinsdedx, fnBinsdedxLE, fnBinsdedxUE);
      hdEdx_Electron_AR->SetTitle("dE/dx (nohad sat) of e- ;dE/dx distrbution (e-);Entries");
      fBasic->Add(hdEdx_Electron_AR);

      TH1D* hdEdx_Positron_AR = new TH1D("hdEdx_Positron_AR", "bla-bla", fnBinsdedx, fnBinsdedxLE, fnBinsdedxUE);
      hdEdx_Positron_AR->SetTitle("dE/dx (nohad sat) of e+;dE/dx distrbution (e+);Entries");
      fBasic->Add(hdEdx_Positron_AR);

      TH2D* hPvsdEdx_AR = new TH2D("hPvsdEdx_AR", "bla-bla", 320, -8.0, 8.0, 100, 0.0, 2.0);
      hPvsdEdx_AR->SetTitle("dE/dx band plots for e+ and e-; Momentum of (e+(right)) and e-);dE/dx");
      fBasic->Add(hPvsdEdx_AR);

      TH2D* hdEdxvsPhi_AR = new TH2D("hdEdxvsPhi_AR", "dE/dx (no had sat) vs #phi", 64, -3.14, 3.14, 200, 0., 2.0);
      hdEdxvsPhi_AR->SetTitle("dE/dx (no Had Sat) vs #phi;track #phi;dE/dx");
      fBasic->Add(hdEdxvsPhi_AR);

      TH2D* hPvsCosth_AR = new TH2D("hPvsCosth_AR", "cos(#theta) vs. p: all Runs", 2 * 48, -10., 10., 60, -1.2, 1.2);
      hPvsCosth_AR->GetXaxis()->SetTitle(Form("Momentum of %s tracks", fCollType.data()));
      hPvsCosth_AR->GetYaxis()->SetTitle("cos(#theta)");
      fBasic->Add(hPvsCosth_AR);


      TH1D* hdEdx_Posi_Pbin_AR[32], *hdEdx_Elec_Pbin_AR[32];
      for (int ip = 0; ip < 32; ip++) {

        hdEdx_Posi_Pbin_AR[ip] = new TH1D(Form("hdEdx_Posi_Pbin_AR%d", ip), Form("hdEdx_Posi_Pbin_AR%d", ip), fnBinsdedx, fnBinsdedxLE,
                                          fnBinsdedxUE);
        hdEdx_Posi_Pbin_AR[ip]->GetXaxis()->SetTitle("dE/dx distrbution (e+)");
        hdEdx_Posi_Pbin_AR[ip]->GetYaxis()->SetTitle("Entries");
        hdEdx_Posi_Pbin_AR[ip]->SetTitle(Form("Momentum range %0.03f to %0.03f", ip * 0.250, (ip + 1) * 0.250));
        fPRdEdxinP->Add(hdEdx_Posi_Pbin_AR[ip]);

        hdEdx_Elec_Pbin_AR[ip] = new TH1D(Form("hdEdx_Elec_Pbin_AR%d", ip), Form("hdEdx_Elec_Pbin_AR%d", ip), fnBinsdedx, fnBinsdedxLE,
                                          fnBinsdedxUE);
        hdEdx_Elec_Pbin_AR[ip]->GetXaxis()->SetTitle("dE/dx distrbution (e-)");
        hdEdx_Elec_Pbin_AR[ip]->GetYaxis()->SetTitle("Entries");
        hdEdx_Elec_Pbin_AR[ip]->SetTitle(Form("Momentum range %0.03f to %0.03f", ip * 0.250, (ip + 1) * 0.250));

      }

      for (int ip = 0; ip < 32; ip++) fPRdEdxinP->Add(hdEdx_Elec_Pbin_AR[ip]); //Adding later for simplicity

      hdEdx_PR.reserve(fnRuns);
    }

    if (fCollType == "hadron") {

      TH2D* hPvsdEdx_hadAR = new TH2D("hPvsdEdx_hadAR", "bla-bla", 500, 0.10, 15.0, 750, 0.05, 15);
      hPvsdEdx_hadAR->SetTitle("dE/dx band plot; Momentum;dE/dx");
      fBasic->Add(hPvsdEdx_hadAR);

      TH2D* hPvsdEdxPion_hadAR = new TH2D("hPvsdEdxPion_hadAR", "bla-bla", 500, 0.10, 15.0, 750, 0.05, 15);
      hPvsdEdxPion_hadAR->SetTitle("dE/dx band plot (Pion); Momentum;dE/dx");
      hPvsdEdxPion_hadAR->SetMarkerColor(kRed);
      fBasic->Add(hPvsdEdxPion_hadAR);

      TH2D* hPvsdEdxKaon_hadAR = new TH2D("hPvsdEdxKaon_hadAR", "bla-bla", 500, 0.10, 15.0, 750, 0.05, 15);
      hPvsdEdxKaon_hadAR->SetTitle("dE/dx band plot (Kaon); Momentum;dE/dx");
      hPvsdEdxKaon_hadAR->SetMarkerColor(kGreen);
      fBasic->Add(hPvsdEdxKaon_hadAR);

      TH2D* hPvsdEdxProton_hadAR = new TH2D("hPvsdEdxProton_hadAR", "bla-bla", 500, 0.10, 15.0, 750, 0.05, 15);
      hPvsdEdxProton_hadAR->SetTitle("dE/dx band plot (Proton); Momentum;dE/dx");
      hPvsdEdxKaon_hadAR->SetMarkerColor(kBlue);
      fBasic->Add(hPvsdEdxProton_hadAR);

      //Pions chi values
      TH1D* hPionChiallP = new TH1D("hPionChiallP", "bla-bla", 240, -6.0, 6.0);
      hPionChiallP->SetTitle("Chi value (Pion);chi value; Entries");
      fBasic->Add(hPionChiallP);

      TH1D* hPionChiLowP = new TH1D("hPionChiLowP", "bla-bla", 240, -6.0, 6.0);
      hPionChiLowP->SetTitle("Chi value (Pion), Momentum (0-300) MeV; chi value; Entries");
      fBasic->Add(hPionChiLowP);

      TH1D* hPionChiHighP = new TH1D("hPionChiHighP", "bla-bla", 240, -6.0, 6.0);
      hPionChiHighP->SetTitle("Chi value (Pion), Momentum (300-400) MeV; chi value; Entries");
      fBasic->Add(hPionChiHighP);

      //Kaons chi values
      TH1D* hKaonChiallP = new TH1D("hKaonChiallP", "bla-bla", 240, -6.0, 6.0);
      hKaonChiallP->SetTitle("Chi value (Kaon);chi value; Entries");
      fBasic->Add(hKaonChiallP);

      TH1D* hKaonChiLowP = new TH1D("hKaonChiLowP", "bla-bla", 240, -6.0, 6.0);
      hKaonChiLowP->SetTitle("Chi value (Kaon), Momentum (0-350) MeV; chi value; Entries");
      fBasic->Add(hKaonChiLowP);

      TH1D* hKaonChiHighP = new TH1D("hKaonChiHighP", "bla-bla", 240, -6.0, 6.0);
      hKaonChiHighP->SetTitle("Chi value (Kaon), Momentum (350-800) MeV; chi value; Entries");
      fBasic->Add(hKaonChiHighP);

      //Protons chi values
      TH1D* hProtonChiallP = new TH1D("hProtonChiallP", "bla-bla", 240, -6.0, 6.0);
      hProtonChiallP->SetTitle("Chi value (Proton);chi value; Entries");
      fBasic->Add(hProtonChiallP);

      TH1D* hProtonChiLowP = new TH1D("hProtonChiLowP", "bla-bla", 240, -6.0, 6.0);
      hProtonChiLowP->SetTitle("Chi value (Proton), Momentum (0-600) MeV; chi value; Entries");
      fBasic->Add(hProtonChiLowP);

      TH1D* hProtonChiHighP = new TH1D("hProtonChiHighP", "bla-bla", 240, -6.0, 6.0);
      hProtonChiHighP->SetTitle("Chi value (Proton), Momentum (600-800) MeV; chi value; Entries");
      fBasic->Add(hProtonChiHighP);
    }
  } else if (level == "PR") {
    if (fCollType != "hadron") {
      hdEdx_PR[iR] = new TH1D(Form("hdEdx_Run%d", fCurrentRunNum), Form("dE/dx (no had sat): Run # = %d", fCurrentRunNum), fnBinsdedx,
                              fnBinsdedxLE, fnBinsdedxUE);
      hdEdx_PR[iR]->GetXaxis()->SetTitle(Form("dE/dx trucMean of %s tracks", fCollType.data()));
      hdEdx_PR[iR]->GetYaxis()->SetTitle("Entries");
      fPRdEdx->Add(hdEdx_PR[iR]);
    }
  } else {
    B2ERROR("Run Gain: Enter AR or PR mode only");
  }
}


//-----------------------------------------------------------------------
void CDCDedxValidationModule::ExtractHistograms(TString level = "exit")
{

  if (level == "PR") {

    Double_t mean = 0., meanError = 0.;
    Double_t sigma = 0., sigmaError = 0.;

    if (hdEdx_PR[fiRun]->GetEntries() > 100) {
      Int_t fitStatus = -1;
      fitStatus = hdEdx_PR[fiRun]->Fit("gaus", "Q"); //Q = No printing
      if (fitStatus == 0) {
        TF1* fit = (TF1*)hdEdx_PR[fiRun]->GetFunction("gaus");
        mean = fit->GetParameter(1);
        meanError = fit->GetParError(1);
        sigma = fit->GetParameter(2);
        sigmaError = fit->GetParError(2);
        hdEdx_PR[fiRun]->GetXaxis()->SetRangeUser(mean - 7 * sigma, mean + 7 * sigma);
        fit->Clear();
      }
    }

    TotRunN.push_back(fCurrentRunNum);
    TotMean.push_back(mean);
    TotMeanE.push_back(meanError);
    TotSigma.push_back(sigma);
    TotSigmaE.push_back(sigmaError);

    if (fiRun % 10 == 0)((TH1D*)fBasic->FindObject("hRunGainPR"))->GetXaxis()->SetBinLabel(fiRun + 1, Form("%d", TotRunN.at(fiRun)));
    ((TH1D*)fBasic->FindObject("hRunGainPR"))->SetBinContent(fiRun + 1, fcRunGain);
    ((TH1D*)fBasic->FindObject("hRunGainPR"))->SetBinError(fiRun + 1, 0.001 * fcRunGain); // no meaning but histogramming only

  } else if (level == "AR") {

    const Int_t allNRun = TotMean.size();
    ((TH1D*)fBasic->FindObject("hRunGainPR"))->GetXaxis()->SetRange(1, allNRun);

    TH1D* hFitdEdxMeanPR = new TH1D("hFitdEdxMeanPR", "dE/dx(nohad-sat) #mu via fit vs. Runs", allNRun, 0, allNRun);
    hFitdEdxMeanPR->GetYaxis()->SetRangeUser(0.90, 1.10);
    hFitdEdxMeanPR->SetMarkerStyle(21);
    hFitdEdxMeanPR->SetMarkerColor(kRed);
    hFitdEdxMeanPR->SetMarkerSize(1);
    hFitdEdxMeanPR->GetXaxis()->SetTitle("Run numbers");
    hFitdEdxMeanPR->GetYaxis()->SetTitle("dEdx mean (fit)");
    hFitdEdxMeanPR->GetXaxis()->LabelsOption("v");

    TH1D* hFitdEdxSigmaPR = new TH1D("hFitdEdxSigmaPR", "dE/dx(nohad-sat) #sigma via fit vs. Runs", allNRun, 0, allNRun);
    hFitdEdxSigmaPR->GetYaxis()->SetRangeUser(0, 0.30);
    hFitdEdxSigmaPR->SetMarkerStyle(21);
    hFitdEdxSigmaPR->SetMarkerColor(kRed);
    hFitdEdxSigmaPR->SetMarkerSize(1);
    hFitdEdxSigmaPR->GetXaxis()->SetTitle("Run numbers");
    hFitdEdxSigmaPR->GetYaxis()->SetTitle("dEdx sigma (fit)");
    hFitdEdxSigmaPR->GetXaxis()->LabelsOption("v");

    for (Int_t i = 0; i < allNRun; i++) {

      if (i % 10 == 0)hFitdEdxMeanPR->GetXaxis()->SetBinLabel(i + 1, Form("%d", TotRunN.at(i)));
      hFitdEdxMeanPR->SetBinContent(i + 1, TotMean.at(i));
      hFitdEdxMeanPR->SetBinError(i + 1, TotMeanE.at(i));

      if (i % 10 == 0)hFitdEdxSigmaPR->GetXaxis()->SetBinLabel(i + 1, Form("%d", TotRunN.at(i)));
      hFitdEdxSigmaPR->SetBinContent(i + 1, TotSigma.at(i));
      hFitdEdxSigmaPR->SetBinError(i + 1, TotSigmaE.at(i));
    }

    fBasic->Add(hFitdEdxMeanPR);
    fBasic->Add(hFitdEdxSigmaPR);

    TH1D* hdEdxFit_allRun = (TH1D*)(fBasic->FindObject(Form("hdEdx_AR"))->Clone("hdEdxFit_allRun"));
    if (hdEdxFit_allRun->GetEntries() > 100) {
      hdEdxFit_allRun->Fit("gaus", "Q");
      TF1* hGfit = (TF1*)hdEdxFit_allRun->GetFunction("gaus");
      Double_t meanGFit = hGfit->GetParameter(1);
      Double_t sigmaGFit = hGfit->GetParameter(2);
      hdEdxFit_allRun->GetXaxis()->SetRangeUser(meanGFit - 7 * sigmaGFit, meanGFit + 7 * sigmaGFit);
      hdEdxFit_allRun->SetFillColor(kYellow);
      hdEdxFit_allRun->SetStats(kTRUE);
      fBasic->Add(hdEdxFit_allRun);
    }

    TH1D* hdEdxMeanVsMomentum = new TH1D("hdEdxMeanVsMomentum", "dEdx-mean vs P bins (BW = 250MeV)", 64, -8.0, 8.0);
    hdEdxMeanVsMomentum->GetXaxis()->SetTitle("Track Momentum");
    hdEdxMeanVsMomentum->GetYaxis()->SetTitle("dEdx Mean");

    TH1D* hdEdxSigmaVsMomentum = new TH1D("hdEdxSigmaVsMomentum", "dEdx-sigma vs P bins (BW = 250MeV)", 64, -8.0, 8.0);
    hdEdxSigmaVsMomentum->GetXaxis()->SetTitle("Track Momentum");
    hdEdxSigmaVsMomentum->GetYaxis()->SetTitle("dEdx Sigma");

    for (int ip = 0; ip < 32; ip++) {
      Int_t nTrack = ((TH1D*)fPRdEdxinP->FindObject(Form("hdEdx_Posi_Pbin_AR%d", ip)))->GetEntries();
      ((TH1D*)fPRdEdxinP->FindObject(Form("hdEdx_Posi_Pbin_AR%d", ip)))->SetFillColor(kYellow);
      Double_t iPMean  = 1.0, iPSigma = 0.0;
      if (nTrack > 100) {
        ((TH1D*)fPRdEdxinP->FindObject(Form("hdEdx_Posi_Pbin_AR%d", ip)))->Fit("gaus", "0");
        iPMean  = ((TH1D*)fPRdEdxinP->FindObject(Form("hdEdx_Posi_Pbin_AR%d", ip)))->GetFunction("gaus")->GetParameter(1);
        iPSigma = ((TH1D*)fPRdEdxinP->FindObject(Form("hdEdx_Posi_Pbin_AR%d", ip)))->GetFunction("gaus")->GetParameter(2);
      }
      hdEdxMeanVsMomentum->SetBinContent(32 + ip + 1, iPMean);
      hdEdxSigmaVsMomentum->SetBinContent(32 + ip + 1, iPSigma);
    }

    for (int ip = 0; ip < 32; ip++) {
      Int_t nTrack = ((TH1D*)fPRdEdxinP->FindObject(Form("hdEdx_Posi_Pbin_AR%d", ip)))->GetEntries();
      ((TH1D*)fPRdEdxinP->FindObject(Form("hdEdx_Elec_Pbin_AR%d", ip)))->SetFillColor(kYellow);
      Double_t iPMean  = 1.0, iPSigma = 0.0;

      if (nTrack > 100) {
        ((TH1D*)fPRdEdxinP->FindObject(Form("hdEdx_Elec_Pbin_AR%d", ip)))->Fit("gaus", "0");
        iPMean  = ((TH1D*)fPRdEdxinP->FindObject(Form("hdEdx_Elec_Pbin_AR%d", ip)))->GetFunction("gaus")->GetParameter(1);
        iPSigma = ((TH1D*)fPRdEdxinP->FindObject(Form("hdEdx_Elec_Pbin_AR%d", ip)))->GetFunction("gaus")->GetParameter(2);
      }
      hdEdxMeanVsMomentum->SetBinContent(32 - ip, iPMean);
      hdEdxSigmaVsMomentum->SetBinContent(32 - ip, iPSigma);
    }

    fBasic->Add(hdEdxSigmaVsMomentum);
    fBasic->Add(hdEdxMeanVsMomentum);
  } else {
    B2ERROR("RunGain >> NO-REQUEST-FOUND for PR or AR level plots, exiting..");
  }
}

//-----------------------------------------
void CDCDedxValidationModule::terminate()
{

  B2INFO("Terminating plots for all runs ");
  fFileOutput = new TFile(Form("fvalidate%s", fOutFileName.data()), "RECREATE");
  if (fCollType != "hadron")ExtractHistograms("AR");
  fFileOutput->cd();
  fBasic->Write("ARBasics", 1);
  if (fCollType == "radbhabha" or fCollType == "bhabha") {
    fPRdEdx->Write("PRdedx", 1);
    fPRdEdxinP->Write("MeanSigmavsP", 1);
  }
  fFileOutput->Close();
}

//-----------------------------------------
Bool_t CDCDedxValidationModule::IsSelectedTrack(const TrackFitResult* mTrack)
{

  if (std::abs(mTrack->getD0()) >= fD0Window || std::abs(mTrack->getZ0()) >= fZ0Window)return kFALSE;
  return kTRUE;

}
