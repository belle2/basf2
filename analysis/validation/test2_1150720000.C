/*
<header>
<input>../1150720000.ntup.root</input>
<output>1150720000_Validation.root</output>
<contact>Matic Lubej; matic.lubej@ijs.si</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_1150720000.C
// Check 
//
// Contributor: Matic Lubej
// June 2, 2017
//
////////////////////////////////////////////////////////////


#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TLegend.h"

void test2_1150720000() {

  gStyle->SetOptStat(0);


  TFile *f = TFile::Open("../1150720000.ntup.root");
  TTree *t = (TTree*) f->Get("rec");

  TH1F *h11 = new TH1F("h11","Signal DeltaE",80,-3,5);
  TH1F *h12 = new TH1F("h12","Signal DeltaE after ROE cleaning",80,-3,5);
  TH1F *h21 = new TH1F("h21","Signal Mbc",80,4.5,5.3);
  TH1F *h22 = new TH1F("h22","Signal Mbc after ROE cleaning",80,4.5,5.3);
  TH1F *h3 = new TH1F("h3","costheta_{BY}",100,-5,5);

  h11->SetLineColor(kRed);
  h21->SetLineColor(kRed);

  h12->SetLineColor(kBlue);
  h22->SetLineColor(kBlue);

  h3->SetLineColor(kBlack);


  t->Project("h11","B0_sigDE0","B0_isSignalAcceptMissingNeutrino == 1");
  t->Project("h12","B0_sigDE1","B0_isSignalAcceptMissingNeutrino == 1");
  t->Project("h21","B0_sigMbc0","B0_isSignalAcceptMissingNeutrino == 1");
  t->Project("h22","B0_sigMbc1","B0_isSignalAcceptMissingNeutrino == 1");
  t->Project("h3","B0_cosThetaBetweenParticleAndTrueB","B0_isSignalAcceptMissingNeutrino == 1");



  TFile* outputFile = new TFile("1150720000_Validation.root","RECREATE");

  outputFile->cd();

  TCanvas *c1 = new TCanvas("c1","pilnu",800,500);
  c1->Divide(3,1);
  
  c1->cd(1);
  h12->Draw();
  h12->GetXaxis()->SetTitle("#DeltaE [GeV]");
  h11->Draw("same");

  TLegend *l1 = new TLegend(0.5,0.7,0.9,0.9);
  l1->AddEntry(h11,"Default","l");
  l1->AddEntry(h12,"After ROE cleanup","l");
  l1->Draw();

  c1->cd(2);
  h22->Draw();
  h22->GetXaxis()->SetTitle("M_{BC} [GeV/c^{2}]");
  h21->Draw("same");

  TLegend *l2 = new TLegend(0.5,0.7,0.9,0.9);
  l2->AddEntry(h21,"Default","l");
  l2->AddEntry(h22,"After ROE cleanup","l");
  l2->Draw();

  c1->cd(3);
  h3->Draw();
  h3->GetXaxis()->SetTitle("cos_{BY}");

  c1->Write();
  outputFile->Close();
  
}
