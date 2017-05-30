/*
<header>
<input>../2411440000.ntup.root</input>
<output>2411440000_Validation.root</output>
<contact>Yubo Li; liyb@pku.edu.cn</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_2411440000.C
// Check the MM2, M_{J/psi} and M_{pipiJ/psi} distributions for
//  e+e- -> pi+pi-J/psi[mu+mu-]   
// Constributor: Yubo Li
// May 27, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"


void plot_pipijpsi(TFile* pfile, TTree* ptree, TFile *outputFile){


  TH1F* h_pipijpsi = new TH1F("h_pipijpsi","M_{#pi#piJ/#psi}",80,3.7,5.4);
  ptree->Project("h_pipijpsi", "vpho_InvM", "1==1");
  h_pipijpsi->GetXaxis()->SetTitle("M_{#pi#piJ/#psi} (GeV/c^{2})");

  TH1F* h_mm2 = new TH1F("h_mm2","MM^{2}_{#pi#piJ/#psi}",100,-1,1);
  ptree->Project("h_mm2", "vpho_m2Recoil", "1==1");
  h_mm2->GetXaxis()->SetTitle("MM^{2}_{#pi#piJ/#psi} (GeV/c^{2})");
  
  TH1F* h_jpsi = new TH1F("h_jpsi","M_{#mu#mu}",100,2.9,3.2);
  ptree->Project("h_jpsi", "vpho_Jpsi_daughterInvariantMass__bo0__cm1__bc", "1==1");
  h_jpsi->GetXaxis()->SetTitle("M_{#mu#mu} (GeV/c^{2})");
  
  outputFile->cd();

  h_pipijpsi->Write();
  h_jpsi->Write();
  h_mm2->Write();
}

void test2_2411440000(){

  TString inputfile("../2411440000.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("h1");

  TFile* outputFile = new TFile("2411440000_Validation.root","RECREATE");

  plot_pipijpsi(sample, tree, outputFile);
  outputFile->Close();

}
