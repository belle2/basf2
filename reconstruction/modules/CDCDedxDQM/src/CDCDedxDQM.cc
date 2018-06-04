/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxDQM/CDCDedxDQM.h>
#include <framework/core/HistoModule.h>
#include <TH1F.h>

using namespace Belle2;


REG_MODULE(CDCDedxDQM)


//---------------------------------
CDCDedxDQMModule::CDCDedxDQMModule(): HistoModule()
{

  setDescription("Make data quality monitoring plots for dE/dx: means and resolutions for bhabha samples, band plots for lepton/hadron samples.");
  addParam("outputFileName", fOutFileName, "Name for output file", string("DefaultdEdxTrendsFile.root"));
  addParam("UsingHadronfiles", isHadronfile, "Switch to hadron (false) vs bhabha files", kFALSE);

}

//---------------------------------
CDCDedxDQMModule::~CDCDedxDQMModule() { }


//---------------------------------
void CDCDedxDQMModule::initialize()
{

  nBinsdedx = 200; nBinsdedxLE = 0.; nBinsdedxUE = 4.;
  nBinsP = 500; nBinsPLE = 0.; nBinsPUE = 10.;
  fCollType = "bhabhaCand";

  if (isHadronfile) {
    nBinsdedx = 500; nBinsdedxLE = 0.; nBinsdedxUE = 10.;
    nBinsP = 150; nBinsPLE = 0.; nBinsPUE = 3.;
    fCollType = "charged";
  }


  fFile = new TFile(Form("%s", fOutFileName.data()), "RECREATE");
  fDir1 = fFile->mkdir("TrendingHisto");
  fDir2 = fFile->mkdir("RunLevelHisto");

  fOutput = new TList();
  fOutput->SetOwner();
  fOutput->SetName("dEdxHistosPlusTrends");

  TH1F* hdEdx_allRun = new TH1F("hdEdx_allRun", "dE/dx Distribution all Runs", nBinsdedx, nBinsdedxLE, nBinsdedxUE);
  hdEdx_allRun->GetXaxis()->SetTitle(Form("dEdx trucMean of %s tracks", fCollType.Data()));
  hdEdx_allRun->GetYaxis()->SetTitle("Entries");
  fOutput->Add(hdEdx_allRun);


  TH2F* hdEdxVsP_allRun = new TH2F("hdEdxVsP_allRun", "dE/dxvs P Distribution all Runs", nBinsP, nBinsPLE, nBinsPUE, nBinsdedx,
                                   nBinsdedxLE, nBinsdedxUE);
  hdEdxVsP_allRun->GetXaxis()->SetTitle(Form("Momentum (P) of %s tracks", fCollType.Data()));
  hdEdxVsP_allRun->GetYaxis()->SetTitle("dEdx");
  fOutput->Add(hdEdxVsP_allRun);
  m_cdcDedxTracks.isRequired();

  REG_HISTOGRAM

}



//---------------------------------
void CDCDedxDQMModule::beginRun()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  fCurrentEventNum = eventMetaDataPtr->getRun();
  //cout << " ----> fCurrentEventNum  = " << Form("Run Number = %d", fCurrentEventNum) << endl;

  TH1F* temp1D = new TH1F(Form("hdEdx_Run%d", fCurrentEventNum), Form("Run Number = %d", fCurrentEventNum), nBinsdedx, nBinsdedxLE,
                          nBinsdedxUE);
  temp1D->GetXaxis()->SetTitle(Form("dEdx trucMean of %s tracks", fCollType.Data()));
  temp1D->GetYaxis()->SetTitle("Entries");
  fOutput->Add(temp1D);


  TH2F* temp2D = new TH2F(Form("hdEdxVsP_Run%d", fCurrentEventNum), Form("Run Number = %d", fCurrentEventNum), nBinsP, nBinsPLE,
                          nBinsPUE, nBinsdedx, nBinsdedxLE, nBinsdedxUE);
  temp2D->GetXaxis()->SetTitle(Form("Momentum (P) of %s tracks", fCollType.Data()));
  temp2D->GetYaxis()->SetTitle("dEdx");
  fOutput->Add(temp2D);

  i1DHistoV.push_back(temp1D);
  i2DHistoV.push_back(temp2D);

}


//---------------------------------
void CDCDedxDQMModule::event()
{

  for (Int_t idedx = 0; idedx < m_cdcDedxTracks.getEntries(); idedx++) {

    CDCDedxTrack* dedxTrack = m_cdcDedxTracks[idedx];
    //per run
    ((TH1F*)(i1DHistoV.at(0)))->Fill(float(dedxTrack->getDedx()));
    ((TH2F*)(i2DHistoV.at(0)))->Fill(float(dedxTrack->getMomentum()), float(dedxTrack->getDedx()));
    //all runs
    ((TH1F*)fOutput->FindObject(Form("hdEdx_allRun")))->Fill(float(dedxTrack->getDedx()));
    ((TH1F*)fOutput->FindObject(Form("hdEdxVsP_allRun")))->Fill(float(dedxTrack->getMomentum()), float(dedxTrack->getDedx()));

  }


}

//---------------------------------
void CDCDedxDQMModule::endRun()
{


  ((TH1F*)(i1DHistoV.at(0)))->Fit("gaus", "Q"); //Q = No printing
  TF1* fit = (TF1*)((i1DHistoV.at(0)))->GetFunction("gaus");
  Double_t mean = fit->GetParameter(1);
  Double_t meanError = fit->GetParError(1);

  Double_t sigma = fit->GetParameter(2);
  Double_t sigmaError = fit->GetParError(2);

  TotRunN.push_back(fCurrentEventNum);
  TotMean.push_back(mean);
  TotMeanE.push_back(meanError);
  TotSigma.push_back(sigma);
  TotSigmaE.push_back(sigmaError);

  i1DHistoV.pop_back();
  i2DHistoV.pop_back();

}



//---------------------------------
void CDCDedxDQMModule::terminate()
{


  const Int_t allNRun = TotMean.size();

  TH1F* hRunVsdEdxMean = new TH1F("hRunVsdEdxMean", "dE/dx mean vs. RunNumber", allNRun, 0, allNRun);
  hRunVsdEdxMean->SetMarkerStyle(21);
  hRunVsdEdxMean->SetMarkerColor(kRed);
  hRunVsdEdxMean->SetMarkerSize(1);
  hRunVsdEdxMean->GetXaxis()->SetTitle("Run Numbers");
  hRunVsdEdxMean->GetYaxis()->SetTitle(Form("dEdx mean of %s", fCollType.Data()));
  hRunVsdEdxMean->Sumw2();

  TH1F* hRunVsdEdxSigma = new TH1F("hRunVsdEdxSigma", "dE/dx sigma vs. RunNumber", allNRun, 0, allNRun);
  hRunVsdEdxSigma->SetMarkerStyle(21);
  hRunVsdEdxSigma->SetMarkerColor(kBlue);
  hRunVsdEdxSigma->SetMarkerSize(1);
  hRunVsdEdxSigma->GetXaxis()->SetTitle("Run Numbers");
  hRunVsdEdxSigma->GetYaxis()->SetTitle(Form("dEdx sigma of %s", fCollType.Data()));
  hRunVsdEdxSigma->Sumw2();

  vector<Double_t> TempTotMean = TotMean;   //Mean of dedx distrbution by Fit
  sort(TempTotMean.begin(), TempTotMean.end());

  for (Int_t i = 0; i < allNRun; i++) {

    hRunVsdEdxMean->GetXaxis()->SetBinLabel(i + 1, Form("%d", TotRunN.at(i)));
    hRunVsdEdxSigma->GetXaxis()->SetBinLabel(i + 1, Form("%d", TotRunN.at(i)));

    if (TotMean.at(i) > 1.15 * TempTotMean.at(int(allNRun / 2)) || TotMean.at(i) < 0.85 * TempTotMean.at(int(allNRun / 2))) {
      hRunVsdEdxMean->GetXaxis()->SetBinLabel(i + 1, Form("#font[22]{#color[2]{%d}}", TotRunN.at(i)));
      hRunVsdEdxSigma->GetXaxis()->SetBinLabel(i + 1, Form("#font[22]{#color[2]{%d}}", TotRunN.at(i)));
    }

    hRunVsdEdxMean->SetBinContent(i + 1, TotMean.at(i));
    hRunVsdEdxMean->SetBinError(i + 1, TotMeanE.at(i));

    hRunVsdEdxSigma->SetBinContent(i + 1, TotSigma.at(i));
    hRunVsdEdxSigma->SetBinError(i + 1, TotSigmaE.at(i));
  }

  //Double_t maxM = hRunVsdEdxMean->GetMaximum();
  hRunVsdEdxMean->SetMinimum(0);
  hRunVsdEdxMean->SetMaximum(TempTotMean.at(int(allNRun / 2)) * 2.0);

  //Double_t maxS = hRunVsdEdxSigma->GetMaximum();
  hRunVsdEdxSigma->SetMinimum(0);
  //hRunVsdEdxSigma->SetMaximum(1);

  fDir1->cd();
  hRunVsdEdxMean->Write();
  hRunVsdEdxSigma->Write();

  fFile->cd();
  fDir2->cd();
  fOutput->Write();
  fFile->Close();
  cout << "--------------Sucessfull-------------- " << endl;

}
