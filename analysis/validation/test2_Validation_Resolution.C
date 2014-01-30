////////////////////////////////////////////////////////////
// Validation_Resolution.cc
// Check the Reconstruction resolution
// Look at single particles, and use truth matching
// 
// Phillip Urquijo
// June 30, 2013
//
#include "TChain.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TF2.h"
#include "TH2F.h"
#include "TH1F.h"
#include <iostream>
#include <fstream>
#include <string>
#include "TTree.h"
#include "TSystem.h"
#include "TROOT.h"
#include <TLorentzVector.h>
#include <TVector3.h>
#include "TGraphAsymmErrors.h"
#include "TLegend.h"
#include "TSystem.h"
#include "TCut.h"
#include "TF1.h"
#include "TH1.h"
#include "TF2.h"
#include "TH2.h"
#include "TMinuit.h"
#include "TGraphErrors.h"
#include "TTree.h"
#include "TFile.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <TStyle.h>
#include "TEventList.h"
#include <TROOT.h>
#include <TSelector.h>
#include <TFractionFitter.h>
#include "THStack.h"
#include "TLegend.h"
#include "TPaveLabel.h"
#include "TFriendElement.h"
#include "TPad.h"
#include "TPaveText.h"
#include "TLatex.h"

#include <string>
#include <TLorentzVector.h>
#include "TMinuit.h"
#include "TObjectTable.h"
#include "TColor.h"
#include "TRandom3.h"
#include "TRolke.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TGraph.h"
#include <TLorentzVector.h>
#include <TVector3.h>

#define NRESBINS 25 
#define NRESMAXP 2.5

const int nthbins=10;
const float thlow=-1.;
const float thhigh=1.;

const int npbins=20;
const int npgbins=24;
const float plow=0.;
const float phigh=1.2;
const float pghigh=2.4;

const int nresbins=50;
const float reslow=-0.05;
const float reshigh=0.05;

const float reswidth=(float)NRESMAXP/(float)NRESBINS;
const float dzreswidth=1/((float)NRESMAXP/(float)NRESBINS);

enum hypo { pion=0, kaon=1, electron=2, muon=3, proton=4, photon=5, ntypes=6};
const int pid[] = {211,321,11,13,2212,22};
const float pdgmasses[] = {0.13957,0.49367,0.000511,0.105658,0.93827,0.};
const char *names[] = { "pi", "k", "e", "mu", "p","gamma"};

//void test2_Validation_Resolution_Tracks(TString,bool);
//void test2_Validation_Resolution_Photons(TString,bool);

void test2_Validation_Resolution(bool runOffline=false){

  if(runOffline){
    SetBelle2Style();
    gROOT->LoadMacro("Belle2Labels.C");
  }

  TString testfile ("../GenericB.ntup.root");
  TCanvas *maincanvas = new TCanvas ("maincanvas","maincanvas");
  maincanvas->Print("resolution.pdf[");
  test2_Validation_Resolution_Tracks(testfile,runOffline);
  test2_Validation_Resolution_Photons(testfile,runOffline);
  maincanvas->Print("resolution.pdf]");
}

void test2_Validation_Resolution_Tracks(TString testfile, bool runOffline){
  
  

  /*  Take the pituple prepared by the NtupleMaker */
  TChain * recoTree = new TChain("pituple");
  recoTree->AddFile(testfile);

  //Lab momentum
  TH1F * h_PpiResolution= new TH1F("P_piResolution"       ,";#sigma(P)/p;N"     ,nresbins,reslow,reshigh);
  //Lab cos(theta)
  TH1F * h_cosThpi      = new TH1F("CosTheta_pi",";cos#theta(#pi) ;N" ,nthbins,thlow,thhigh);	
  
  TH1F * h_PpiRes[NRESBINS];
  TH1F * h_dzRes[NRESBINS];
  for(int ipbin=0;ipbin<NRESBINS;ipbin++)
    h_PpiRes[ipbin]= new TH1F(Form("P_piRes_%d", ipbin),
			      ";p_{T}(#pi) GeV;Arbitrary Normalisation" ,200,-0.05,.05);

  TH2F * h_Resolution   = new TH2F("Resolution" ,";p_{T}(#pi) GeV;#sigma(p_{T})/p_{T}"           ,npbins,plow,phigh,10,0.001, .5);
  TH2F * h_Resolutionc   = new TH2F("Resolutionc" ,";p_{T}(#pi) GeV;Mean Bias (Rec-True)/Mean"   ,npbins,plow,phigh,10,-0.005, 0.015);
 
  for(int ipbin=0;ipbin<NRESBINS;ipbin++)
    h_dzRes[ipbin]= new TH1F(Form("dz_Res_%d", ipbin),";dz;Arbitrary Normalisation"       ,100,0.,1000.);
  
  TH2F * h_dzResolution   = new TH2F("dzResolution" ,";p_{T};#sigma(dz) #mu m"            ,npbins,plow,phigh,10,0., 200);

  float fpi_TruthP;  
  float fpi_P4[4];  
  float fpi_TruthP4[4];  
  float fpi_PIDpi;  
  float fpi_PIDk;  
  float fpi_PIDe;  
  float fpi_PIDmu;  
  float fpi_PIDp;  
  float fpi_dz;  
  int ipi_mcID;  
  recoTree->SetBranchAddress("pi_TruthP",  &fpi_TruthP);  
  recoTree->SetBranchAddress("pi_mcID", &ipi_mcID);  
  recoTree->SetBranchAddress("pi_TruthP4",      &fpi_TruthP4);  
  recoTree->SetBranchAddress("pi_P4",      &fpi_P4);  
  recoTree->SetBranchAddress("pi_PIDpi",   &fpi_PIDpi);  
  recoTree->SetBranchAddress("pi_PIDk",    &fpi_PIDk);  
  recoTree->SetBranchAddress("pi_PIDe",    &fpi_PIDe);  
  recoTree->SetBranchAddress("pi_PIDmu",   &fpi_PIDmu);  
  recoTree->SetBranchAddress("pi_PIDp",    &fpi_PIDp);  
  recoTree->SetBranchAddress("pi_dz",    &fpi_dz);  
  
  for(Int_t iloop=0;iloop<recoTree->GetEntries();iloop++) {
    recoTree->GetEntry(iloop);
    TLorentzVector lv_pi(fpi_P4);  
    TLorentzVector lv_pi_truth(fpi_TruthP4);  
    int pdgid=abs(ipi_mcID);
    
    if(pdgid==pid[pion]){
      h_PpiResolution->Fill((lv_pi.Pt()-lv_pi_truth.Pt())/lv_pi_truth.Pt());
      h_cosThpi->Fill(lv_pi.CosTheta());
      
      for(int ipbin=0;ipbin<NRESBINS;ipbin++)
	if(lv_pi_truth.Pt()>=(float)ipbin*reswidth && lv_pi_truth.Pt()<((float)ipbin+1)*reswidth)
	  h_PpiRes[ipbin]->Fill(lv_pi.Pt()-lv_pi_truth.Pt());
      
      for(int ipbin=0;ipbin<NRESBINS;ipbin++)
	if(lv_pi_truth.Pt()>=(float)ipbin*reswidth && lv_pi_truth.Pt()<((float)ipbin+1)*reswidth)h_dzRes[ipbin]->Fill(fpi_dz*1000);
      
      }
      
    }

  TCanvas *tc = new TCanvas ("tc","tcReco");
  tc->cd()->SetLogy(1);
  h_PpiResolution->SetLineColor(kRed);
  h_PpiResolution->Draw();
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");

  tc->cd()->SetLogy(0);
  h_cosThpi->SetLineColor(kRed);
  h_cosThpi->Draw();
  
  tc->Print("resolution.pdf","Title: Momentum Resolution");
  
  double xpoints[NRESBINS], ypoints[NRESBINS], xepoints[NRESBINS], yepoints[NRESBINS];
  double ypointsmean[NRESBINS], yepointsmean[NRESBINS];
  
  for(int ipbin=0;ipbin<NRESBINS;ipbin++){
    h_PpiRes[ipbin]->Scale(1/h_PpiRes[ipbin]->Integral());
    xpoints[ipbin] = (float)ipbin*reswidth+reswidth/2.;
    ypoints[ipbin] = h_PpiRes[ipbin]->GetRMS()/xpoints[ipbin];
    ypointsmean[ipbin] = h_PpiRes[ipbin]->GetMean()/xpoints[ipbin];
    xepoints[ipbin] = reswidth/2.;
    yepoints[ipbin] = h_PpiRes[ipbin]->GetRMSError()/xpoints[ipbin];
    yepointsmean[ipbin] = h_PpiRes[ipbin]->GetMeanError()/xpoints[ipbin];
  }
  
  h_PpiRes[NRESBINS-1]->SetLineColor(NRESBINS);
  h_PpiRes[NRESBINS-1]->Draw("");
  
  for(int ipbin=NRESBINS-2;ipbin>-1;ipbin--){
    h_PpiRes[ipbin]->SetLineColor(ipbin+1);
    h_PpiRes[ipbin]->Draw("same");
  }
  
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  tc->Print("resolution.pdf","Title: 1/pT in bins of pT");
  
  tc->Clear();
  tc->Divide(2,2);
  for(int i=0;i<NRESBINS;i++){
    tc->cd(i%4+1);
    h_PpiRes[i]->SetLineColor(kBlue);
    h_PpiRes[i]->Draw();
    if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
    if((i+1)%4==0)  tc->Print("resolution.pdf",Form("Title:  1/pT in bins of pT %d",(i+1)/4));
  }
  
  
  tc->Clear();
  tc->cd()->SetLogy(1);
  tc->cd()->SetGridy(1);
  tc->cd()->SetGridx(1);
  TGraphErrors *tg = new TGraphErrors(NRESBINS,xpoints,ypoints,xepoints,yepoints);
  h_Resolution->Draw();
  tg->SetMinimum(0.001);
  tg->SetFillColor(kBlue);
  tg->SetMarkerColor(kBlue);
  tg->SetMarkerStyle(21);
  tg->Draw("2p");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  tc->Print("resolution.pdf","Title: Resolution in bins of pT");
  tc->Clear();
  tc->cd()->SetLogy(0);
  
  TGraphErrors *tg2 = new TGraphErrors(NRESBINS,xpoints,ypointsmean,xepoints,yepointsmean);
  h_Resolutionc->Draw();
  tg2->SetMinimum(0.001);
  tg2->SetFillColor(kBlue);
  tg2->Draw("2");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  tc->Print("resolution.pdf","Title: Resolution in bins of pT");
  tc->Clear();

  for(int ipbin=0;ipbin<NRESBINS;ipbin++){
    cout<<h_dzRes[ipbin]->GetRMS()<<" "<<h_dzRes[ipbin]->GetRMSError()<<endl;
    xpoints[ipbin] = (float)ipbin*reswidth+reswidth/2.;
    ypoints[ipbin] = h_dzRes[ipbin]->GetRMS();
    xepoints[ipbin] = reswidth/2.;
    yepoints[ipbin] = h_dzRes[ipbin]->GetRMSError();
  }
  

  TFile* output = new TFile("ResolutionValidationTracks.root", "recreate");

  //Resolution plot of integrated data.
  TH1F *h_output_PpiResolution = (TH1F*)h_PpiResolution->Clone("P_output_piResolution");
  h_output_PpiResolution->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction resolution of truth-matched pion tracks with a pi hypothesis in bins of transverse momentum. A Generic BBbar sample is used."));
  h_output_PpiResolution->GetListOfFunctions()->Add(new TNamed("Check", "Consistent width and mean."));
  
  //Resolution in bins of pT
  TH1F * h_outputResolution   = new TH1F("outputResolution" ,";p_{T}(#pi) GeV;#sigma(p_{T})/p_{T}"           ,npbins,plow,phigh);
  h_outputResolution->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction resolution of truth-matched pion tracks with a pi hypothesis in bins of transverse momentum. Sigma is determined from getRMS. A Generic BBbar sample is used."));
  h_outputResolution->GetListOfFunctions()->Add(new TNamed("Check", "Stable resolution."));

  for(int i=0;i<NRESBINS;i++){
    double tgx, tgy, tgye;
    tg->GetPoint(i,tgx,tgy);
    tgye=tg->GetErrorY(i);
    h_outputResolution->SetBinContent(i+1,tgy);
    h_outputResolution->SetBinError(i+1,tgye);
  }

  //Bias in bins of pT

  TH1F * h_outputBias   = new TH1F("outputBias" ,";p_{T}(#pi) GeV;Mean Bias (Rec-True)/Mean"   ,npbins,plow,phigh);
  h_outputBias->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction pT bias of truth-matched pion tracks with a pi hypothesis in bins of transverse momentum. The relative bias, i.e. (Mean(Rec)-Mean(True))/Mean is shown. A Generic BBbar sample is used."));
  h_outputBias->GetListOfFunctions()->Add(new TNamed("Check", "Stable resolution. Low pT tracks < 100 MeV exhibit larger relative bias."));

  for(int i=0;i<NRESBINS;i++){
    double tgx, tgy, tgye;
    tg2->GetPoint(i,tgx,tgy);
    tgye=tg2->GetErrorY(i);
    h_outputBias->SetBinContent(i+1,tgy);
    h_outputBias->SetBinError(i+1,tgye);
  }


  output->Write();
  output->Close();
 
}


void test2_Validation_Resolution_Photons(TString testfile,bool runOffline){
  
  TChain * recoTree = new TChain("gammatuple");
  recoTree->AddFile(testfile);
  TH1F * h_Resolution[NRESBINS];
  for(int i=0;i<NRESBINS;i++)h_Resolution[i]= new TH1F(Form("Resolution%d",i), ";E(#gamma) Rec-Truth;" ,100,-0.1,0.04);
  TH2F * h_Resolution_2D = new TH2F("Resolution2D", ";E(#gamma) Rec GeV;E(#gamma) Truth GeV"     ,100,0.,3.0,100,0.,3.0);
  TH2F * h_Resolutionb   = new TH2F("Resolutionb" ,";E (#gamma) GeV;#sigma(E)/E"           ,npgbins,plow,pghigh,10,0., .25);
  TH2F * h_Resolutionc   = new TH2F("Resolutionc" ,";E (#gamma) GeV;Mean Bias (Rec-Truth)/Mean"  ,npgbins,plow,pghigh,10,-0.1, 0.0);

  
  float fgamma_TruthP;  
  float fgamma_P;  
  int igamma_mcID;  
  
  recoTree->SetBranchAddress("gamma_mcID", &igamma_mcID);  
  recoTree->SetBranchAddress("gamma_TruthP", &fgamma_TruthP);  
  recoTree->SetBranchAddress("gamma_P",      &fgamma_P);  
  
  for(Int_t iloop=0;iloop<recoTree->GetEntries();iloop++) {
    
    recoTree->GetEntry(iloop);
    int pdgid=abs(igamma_mcID);
    if(pdgid==22){
      for(int i=0;i<NRESBINS;i++)if(fgamma_TruthP>=i*reswidth && fgamma_TruthP<(i+1)*reswidth)h_Resolution[i]->Fill((fgamma_P-fgamma_TruthP));
      h_Resolution_2D->Fill(fgamma_P,fgamma_TruthP);
    }
  }


  
  TCanvas *tc = new TCanvas ("tc","tcReco");
  tc->Divide(2,2);
  for(int i=0;i<NRESBINS;i++){
    tc->cd(i%4+1);
    h_Resolution[i]->Draw();
    cout<<h_Resolution[i]->GetMean()<<" "<<h_Resolution[i]->GetRMS()<<endl;
    if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
    if((i+1)%4==0)  tc->Print("resolution.pdf",Form("Title: Photon energy resolution %d",(i+1)/4));
  }
  
  double xpoints[NRESBINS], ypoints[NRESBINS], xepoints[NRESBINS], yepoints[NRESBINS];

  double ypointsmean[NRESBINS],yepointsmean[NRESBINS];
 
  for(int ipbin=0;ipbin<NRESBINS;ipbin++){
    h_Resolution[ipbin]->Scale(1/h_Resolution[ipbin]->Integral());
    xpoints[ipbin] = (float)ipbin*reswidth+reswidth/2.;
    ypoints[ipbin] = h_Resolution[ipbin]->GetRMS()/xpoints[ipbin];
    ypointsmean[ipbin] = h_Resolution[ipbin]->GetMean()/xpoints[ipbin];
    cout<<"ypoints res "<<ypoints[ipbin]<<endl;

    xepoints[ipbin] = reswidth/2.;
    yepoints[ipbin] = h_Resolution[ipbin]->GetRMSError()/xpoints[ipbin];
    yepointsmean[ipbin] = h_Resolution[ipbin]->GetMeanError()/xpoints[ipbin];
  }
  tc->Clear();
  // tc->cd()->SetLogy(1);
  tc->cd()->SetGridy(1);
  tc->cd()->SetGridx(1);
  TGraphErrors *tg = new TGraphErrors(NRESBINS,xpoints,ypoints,xepoints,yepoints);
  h_Resolutionb->SetMinimum(0.00001);
  h_Resolutionb->Draw();
  tc->Update();
  tg->SetMinimum(0.0001);
  tg->SetFillColor(kBlue);
  tg->SetMarkerColor(kBlue);
  tg->SetMarkerStyle(21);
  tg->Draw("2p");
  tc->Update();
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  tc->Print("resolution.pdf","Title: Detector resolution for photons");
  
  tc->Clear();
  tc->cd()->SetLogy(0);

  TGraphErrors *tg2 = new TGraphErrors(NRESBINS,xpoints,ypointsmean,xepoints,yepointsmean);
  h_Resolutionc->Draw();
  tg2->SetFillColor(kBlue);
  tg2->Draw("2");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  tc->Print("resolution.pdf","Title: Detector resolution for photons");


  tc->Clear();
  h_Resolution_2D->Draw("box");
  if(runOffline)BELLE2Label(0.4,0.9,"Simulation (Preliminary)");
  tc->Print("resolution.pdf","Title: Detector response for photons");
  
  ////////////////////////////

  TFile* output = new TFile("ResolutionValidationPhotons.root", "recreate");

  //Resolution in bins of E
  TH1F * h_outputResolution   = new TH1F("outputResolutionb" ,";E (#gamma) GeV;#sigma(E)/sqrt(E)" ,npgbins,plow,pghigh);
  h_outputResolution->GetListOfFunctions()->Add(new TNamed("Description", "Single photon reconstruction resolution of truth-matched photons in bins of true energy. Sigma is determined from getRMS. A Generic BBbar sample is used."));
  h_outputResolution->GetListOfFunctions()->Add(new TNamed("Check", "Stable resolution. Broad at low energy, flattens to <0.2 after E>0.2 GeV"));

  for(int i=0;i<npgbins;i++){
    double tgx, tgy, tgye;
    tg->GetPoint(i,tgx,tgy);
    tgye=tg->GetErrorY(i);
    h_outputResolution->SetBinContent(i+1,tgy);
    h_outputResolution->SetBinError(i+1,tgye);
  }

  //Bias in bins of pT

  TH1F * h_outputBias   = new TH1F("outputBiasb" ,";E (#gamma) GeV;Mean Bias (Rec-True)/Mean"   ,npgbins,plow,phigh);
  h_outputBias->GetListOfFunctions()->Add(new TNamed("Description", "Single photon reconstruction Energy bias of truth-matched photons in bins of true energy. The relative bias, i.e. (Mean(Rec)-Mean(True))/Mean is shown. A Generic BBbar sample is used."));
  h_outputBias->GetListOfFunctions()->Add(new TNamed("Check", "Stable resolution. Low E tracks < 200 MeV exhibit larger relative bias up to 10%."));

  for(int i=0;i<npgbins;i++){
    double tgx, tgy, tgye;
    tg2->GetPoint(i,tgx,tgy);
    tgye=tg2->GetErrorY(i);
    h_outputBias->SetBinContent(i+1,tgy);
    h_outputBias->SetBinError(i+1,tgye);
  }


  output->Write();
  output->Close();
}
