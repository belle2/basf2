/*
<header>
<input>../1213012101.ntup.root</input>
<output>1213012101_Validation.root</output>
<contact>Minakshi Nayak; minakshi.nayak@wayne.edu</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_1213012101.C
// Check Mbc, DeltaE, MD, MKs, and Mpi0
//
// Constributor: Minakshi Nayak
// May 17, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

/* Validation script to determine benchmarks for PID cuts. */

void plot(TFile* pfile, TTree* ptree, TFile *outputFile){


  TString tmCuts("(B__isSignal == 1)");

  TH1F* h_mbc = new TH1F("h_mbc","M_{bc}",100,5.25,5.29);
  ptree->Project("h_mbc", "B_mbc", tmCuts);
  h_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");


  TH1F* h_deltae = new TH1F("h_deltae","#DeltaE",100,-0.2,0.15);
  ptree->Project("h_deltae", "B_deltae", tmCuts);
  h_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)");

  TH1F* h_MKs = new TH1F("h_MKs","M_{Ks}",100,0.45,0.55);
  ptree->Project("h_MKs", "B_D0_K_S0_M", tmCuts);
  h_MKs->GetXaxis()->SetTitle("M_{Ks} (GeV/c^{2})");

  TH1F* h_Mpi0 = new TH1F("h_Mpi0","M_{pi0}",100,0.096,0.164);
  ptree->Project("h_Mpi0", "B_D0_pi0_M", tmCuts);
  h_Mpi0->GetXaxis()->SetTitle("M_{pi0} (GeV/c^{2})");


  TH1F* h_MD = new TH1F("h_MD","M_{D}",100,1.75,1.95);
  ptree->Project("h_MD", "B_D0_M", tmCuts);
  h_MD->GetXaxis()->SetTitle("M_{D} (GeV/c^{2})");

  outputFile->cd();

  h_mbc->Write();
  h_deltae->Write();
  h_MKs->Write();
  h_Mpi0->Write();
  h_MD->Write();
}




void test2_1213012101(){

  TString inputfile("../1213012101.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("bdk");

  TFile* outputFile = new TFile("1213012101_Validation.root","RECREATE");

  plot(sample, tree, outputFile);


  outputFile->Close();

}
