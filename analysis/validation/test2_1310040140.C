/*
<header>
<input>../1310040140.ntup.root</input>
<output>1310040140_Validation.root</output>
<contact>Bilas Pal; palbs@ucmail.uc.edu</contact>
<description> Reference plots for Bs->KsKs analysis</descrtiption>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_1213012101.C
// Check Mbc, DeltaE,  MKs
//
// Constributor: Bilas Pal
// May 31, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

/* Validation script to determine benchmarks for PID cuts. */

void plot(TFile* pfile, TTree* ptree, TFile *outputFile){


  TString tmCuts("(B_s0_isSignal == 1)");

  TH1F* h_mbc = new TH1F("h_mbc","M_{bc} (for B_{s}#rightarrow K_s K_s)",100,5.34,5.44);
  ptree->Project("h_mbc", "B_s0_mbc", tmCuts);
  h_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");


  TH1F* h_deltae = new TH1F("h_deltae","#DeltaE (for B_{s}#rightarrow K_s K_s)",100,-0.2,0.1);
  ptree->Project("h_deltae", "B_s0_deltae", tmCuts);
  h_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)");

  TH1F* h_MKs = new TH1F("h_MKs","M_{Ks} (for B_{s}#rightarrow K_s K_s)",100,0.45,0.55);
  ptree->Project("h_MKs", "B_s0_K_S00_M", tmCuts);
  h_MKs->GetXaxis()->SetTitle("M_{Ks} (GeV/c^{2})");

  TNamed* contact = new TNamed("Contact", "bilas.kanti.pal@desy.de");

  h_mbc->GetListOfFunctions()->Add(new TNamed("Description", h_mbc->GetTitle()));
  h_mbc->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  h_mbc->GetListOfFunctions()->Add(contact);
  
  h_deltae->GetListOfFunctions()->Add(new TNamed("Description", h_deltae->GetTitle()));
  h_deltae->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  h_deltae->GetListOfFunctions()->Add(contact);
  
  h_MKs->GetListOfFunctions()->Add(new TNamed("Description", h_MKs->GetTitle()));
  h_MKs->GetListOfFunctions()->Add(new TNamed("Check", "Check position and width of the peak"));
  h_MKs->GetListOfFunctions()->Add(contact);

  outputFile->cd();

  h_mbc->Write();
  h_deltae->Write();
  h_MKs->Write();
}


void test2_1310040140(){
   
  TString inputfile("../1310040140.ntup.root");
  
  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("Bs");

  TFile* outputFile = new TFile("1310040140_Validation.root","RECREATE");

  plot(sample, tree, outputFile);


  outputFile->Close();

}
