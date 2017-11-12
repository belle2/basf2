/*
<header>
<input>../1150720000.ntup.root</input>
<output>1150720000_Validation.root</output>
<contact>Mario Merola; mario.merola@na.infn.it</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_1150720000.C
// Check Mbc, DeltaE and costheta_BY for the ROE of B - >pi l nu
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

  TH1F *h_deltae = new TH1F("h_deltae","#DeltaE (B^{0} #rightarrow #pi l #nu)",30,-3,5);
  TH1F *h_deltae_clean = new TH1F("h_deltae_clean","#DeltaE (B^{0} #rightarrow #pi l #nu)",30,-3,5);
  TH1F *h_mbc = new TH1F("h_mbc","M_{bc} (B^{0} #rightarrow #pi l #nu)",30,4.5,5.3);
  TH1F *h_mbc_clean = new TH1F("h_mbc_clean","M_{bc} (B^{0} #rightarrow #pi l #nu)",30,4.5,5.3);
  TH1F *h_costheta = new TH1F("h_costheta","costheta_{BY}",30,-5,5);

  h_deltae->SetLineColor(kRed);
  h_mbc->SetLineColor(kRed);

  h_deltae_clean->SetLineColor(kBlue);
  h_mbc_clean->SetLineColor(kBlue);

  h_costheta->SetLineColor(kBlack);



  t->Project("h_deltae","B0_sigDE0","B0_isSignalAcceptMissingNeutrino == 1");
  t->Project("h_deltae_clean","B0_sigDE1","B0_isSignalAcceptMissingNeutrino == 1");
  t->Project("h_mbc","B0_sigMbc0","B0_isSignalAcceptMissingNeutrino == 1");
  t->Project("h_mbc_clean","B0_sigMbc1","B0_isSignalAcceptMissingNeutrino == 1");
  t->Project("h_costheta","B0_cosThetaBetweenParticleAndTrueB","B0_isSignalAcceptMissingNeutrino == 1");

  h_deltae->GetListOfFunctions()->Add(new TNamed("Description", "B^{0} #rightarrow #pi l #nu  #DeltaE [GeV]"));
  h_deltae->GetListOfFunctions()->Add(new TNamed("Contact" , "mario.merola@na.infn.it"));
  h_deltae->GetListOfFunctions()->Add(new TNamed("Check", "#DeltaE is expected to peak at 0 GeV"));

  h_deltae_clean->GetListOfFunctions()->Add(new TNamed("Description", "B^{0} #rightarrow #pi l #nu  #DeltaE [GeV] after Rest Of Event cleaning"));
  h_deltae_clean->GetListOfFunctions()->Add(new TNamed("Contact" , "mario.merola@na.infn.it"));
  h_deltae_clean->GetListOfFunctions()->Add(new TNamed("Check", "#DeltaE is expected to peak at 0 GeV"));

  h_mbc->GetListOfFunctions()->Add(new TNamed("Description", "B^{0} #rightarrow #pi l #nu  M_{bc} [GeV/c^{2}]"));
  h_mbc->GetListOfFunctions()->Add(new TNamed("Contact" , "mario.merola@na.infn.it"));
  h_mbc->GetListOfFunctions()->Add(new TNamed("Check", "M_{bc} is expected to peak at 5.279 GeV/c^{2}"));

  h_mbc_clean->GetListOfFunctions()->Add(new TNamed("Description", "B^{0} #rightarrow #pi l #nu  M_{bc} [GeV/c^{2}] after Rest Of Event cleaning"));
  h_mbc_clean->GetListOfFunctions()->Add(new TNamed("Contact" , "mario.merola@na.infn.it"));
  h_mbc_clean->GetListOfFunctions()->Add(new TNamed("Check", "M_{bc} is expected to peak at 5.279 GeV/c^{2}"));





  TFile* outputFile = new TFile("1150720000_Validation.root","RECREATE");

  outputFile->cd();

  TCanvas *c1 = new TCanvas("c1","pilnu",800,500);
  c1->Divide(3,1);
  
  c1->cd(1);
  h_deltae_clean->Draw();
  h_deltae_clean->GetXaxis()->SetTitle("#DeltaE [GeV]");
  h_deltae->GetXaxis()->SetTitle("#DeltaE [GeV]");
  h_deltae->Draw("same");

  TLegend *l1 = new TLegend(0.5,0.7,0.9,0.9);
  l1->AddEntry(h_deltae,"Default","l");
  l1->AddEntry(h_deltae_clean,"After ROE cleanup","l");
  l1->Draw();

  c1->cd(2);
  h_mbc_clean->Draw();
  h_mbc_clean->GetXaxis()->SetTitle("M_{BC} [GeV/c^{2}]");
  h_mbc->GetXaxis()->SetTitle("M_{BC} [GeV/c^{2}]");
  h_mbc->Draw("same");

  TLegend *l2 = new TLegend(0.5,0.7,0.9,0.9);
  l2->AddEntry(h_mbc,"Default","l");
  l2->AddEntry(h_mbc_clean,"After ROE cleanup","l");
  l2->Draw();

  c1->cd(3);
  h_costheta->Draw();
  h_costheta->GetXaxis()->SetTitle("cos_{BY}");


  h_mbc_clean->SetTitle("M_{bc} after ROE cleaning (B^{0} #rightarrow #pi l #nu)");
  h_deltae_clean->SetTitle("#DeltaE after ROE cleaning (B^{0} #rightarrow #pi l #nu)");

  h_mbc->Write();
  h_mbc_clean->Write();
  h_deltae->Write();
  h_deltae_clean->Write();

  c1->Write();
  outputFile->Close();
  
}
