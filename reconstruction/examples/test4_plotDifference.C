#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"

void test4_plotDifference(){

  TFile* uncorrected = new TFile("B2Electrons.root");
  TTree* untree = (TTree*)uncorrected->Get("tree");
  uncorrected->cd();
  untree->Project("unhist(100,0,4)","CDCDedxTracks.h_dedx");
  TH1F* unhist = (TH1F*)uncorrected->Get("unhist");
  unhist->SetTitle("Uncorrected;dE/dx;");
  
  TFile* corrected = new TFile("NewB2Electrons.root");
  TTree* tree = (TTree*)corrected->Get("tree");
  corrected->cd();
  tree->Project("hist(100,0,4)","CDCDedxTracks.h_dedx");
  TH1F* hist = (TH1F*)corrected->Get("hist");
  hist->SetTitle("Corrected;dE/dx;");
  
  TCanvas* can = new TCanvas("can","",1000,600);
  can->Divide(2,1);
  can->cd(1);
  unhist->Draw();
  can->cd(2);
  hist->SetLineColor(kRed);
  hist->Draw();
}
