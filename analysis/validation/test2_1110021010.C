/*
<header>
<input>../1110021010.ntup.root</input>
<output>1110021010_Validation.root</output>
<contact>Saurabh Sandilya; saurabhsandilya@gmail.com</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_1110021010.C
// Check the Delta E and Mbc distributions for
//  B0->rho0[pi+pi-]gamma
// Constributor: Saurabh Sandilya
// May 17, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

/* Validation script to determine benchmarks for PID cuts. */

void plot_dembc(TFile* pfile, TTree* ptree, TFile *outputFile){

  TString tmCuts("(B0_rho0_mcPDG==113&&B0_gamma_mcPDG==22)");

  TH1F* h_mbc = new TH1F("h_mbc","M_{bc}",180,5.2,5.29);
  ptree->Project("h_mbc", "B0_mbc", "1==1");
  h_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");

  TH1F* h_mbc_T = new TH1F("h_mbc_T","M_{bc}",180,5.2,5.29);
  ptree->Project("h_mbc_T", "B0_mbc", tmCuts);
  h_mbc_T->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");
  
  TH1F* h_deltae = new TH1F("h_deltae","#DeltaE",100,-0.5,0.5);
  ptree->Project("h_deltae", "B0_deltae", "1==1");
  h_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)");

  TH1F* h_deltae_T = new TH1F("h_deltae_T","#DeltaE",100,-0.5,0.5);
  ptree->Project("h_deltae_T", "B0_deltae", tmCuts);
  h_deltae_T->GetXaxis()->SetTitle("#DeltaE (GeV)");

  TH1F* h_egam = new TH1F("h_egam","E_{lab}(#gamma)",250,1.5,4.0);
  ptree->Project("h_egam", "B0_gamma_P", "1==1");
  h_egam->GetXaxis()->SetTitle("E_{lab}(#gamma) (GeV)");

  
  outputFile->cd();

  h_mbc->Write();
  h_deltae->Write();
  h_egam->Write();
}

void test2_1110021010(){

  TString inputfile("../1110021010.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("h1");

  TFile* outputFile = new TFile("1110021010_Validation.root","RECREATE");

  plot_dembc(sample, tree, outputFile);
  outputFile->Close();

}
