//////////////////////////////////////////////////////////
// 5 - plot
//
// Plot the results of the calibration and compare
//
// Usage: root calib5_plotDifference.C
//
// Input: B2Electrons.root, NewB2Electrons.root
// Output: none
//
// Contributors: Jake Bennett
//////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"

void calib5_plotDifference(){

  TFile* uncorrected = new TFile("B2Electrons.root");
  TTree* untree = (TTree*)uncorrected->Get("tree");
  uncorrected->cd();
  untree->Project("unhist(100,0,4)","CDCDedxTracks.h_dedx");
  TH1F* unhist = (TH1F*)uncorrected->Get("unhist");
  unhist->SetTitle("Uncorrected;dE/dx;");

  untree->Project("unhistcos(100,-1,1,100,0,4)","CDCDedxTracks.h_dedx:CDCDedxTracks.m_cosTheta");
  TH2F* unhistcos = (TH2F*)uncorrected->Get("unhistcos");
  unhistcos->SetTitle("Uncorrected;dE/dx;");

  TFile* corrected = new TFile("NewB2Electrons.root");
  TTree* tree = (TTree*)corrected->Get("tree");
  corrected->cd();
  tree->Project("hist(100,0,4)","CDCDedxTracks.h_dedx");
  TH1F* hist = (TH1F*)corrected->Get("hist");
  hist->SetTitle("Corrected;dE/dx;");

  tree->Project("histcos(100,-1,1,100,0,4)","CDCDedxTracks.h_dedx:CDCDedxTracks.m_cosTheta");
  TH2F* histcos = (TH2F*)corrected->Get("histcos");
  histcos->SetTitle("Corrected;dE/dx;");

  TCanvas* can = new TCanvas("can","",1000,600);
  can->Divide(2,1);
  can->cd(1);
  unhist->Draw();
  can->cd(2);
  hist->SetLineColor(kRed);
  hist->Draw();

  TCanvas* can2 = new TCanvas("can2","",1000,600);
  can2->Divide(2,1);
  can2->cd(1);
  unhistcos->Draw("colz");
  can2->cd(2);
  histcos->Draw("colz");
}
