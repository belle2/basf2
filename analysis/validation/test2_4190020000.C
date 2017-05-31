
/*
<header>
<input>../4190020000.ntup.root</input>
<output>4190020000_Validation.root</output>
<contact>Suxian Li; lisuxian@buaa.edu.cn</contact>
</header>
*/

//////////////////////////////////////////////////////////
//
// test2_4190020000.C
// Check 
//
// Constributor: Suxian Li
// May 30, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

/* Validation script to determine benchmarks for PID cuts. */
void plot_Mrecoil_pipi(TFile* pfile, TTree* ptree, TFile* outputFile){
 
   TString pidCut("Upsilon6S_pi0_PIDpi > 0.01 && Upsilon6S_pi1_PIDpi > 0.01");
   TString mcCuts("(Upsilon6S_pi0_mcPDG ==211 ) && (Upsilon6S_pi1_mcPDG == -211)");
   TString Cuts(pidCut+"&&"+mcCuts);
     
   TH1F* h_Mrecoil_pipi = new TH1F("h_Mrecoil_pipi","Mrecoil(pipi)",450,9.6,10.5);
   ptree->Project("h_Mrecoil_pipi","Upsilon6S_Mrecoil",Cuts);
   h_Mrecoil_pipi->GetXaxis()->SetTitle("M_{recoil}(#pi^{+}#pi^{-}) (GeV/c^{2})");

   outputFile->cd();
 
   h_Mrecoil_pipi->Write();

}

void plot_Mrecoil_pi(TFile* pfile, TTree* ptree, TFile* outputFile){

   TString pidCut("Upsilon6S_pi_PIDpi > 0.01");
   TString nHits("Upsilon6S_pi_nPXDHits > 0");
   TString Cuts(pidCut+"&&"+nHits);
 
   TH1F* h_Mrecoil_pi = new TH1F("h_Mrecoil_pi","Mrecoil(pi)",280,9.5,10.9);
   ptree->Project("h_Mrecoil_pi","Upsilon6S_Mrecoil",Cuts);
   h_Mrecoil_pi->GetXaxis()->SetTitle("M_{recoil}(#pi^{+}) (GeV/c^{2})");

   outputFile->cd();

   h_Mrecoil_pi->Write();

}

void test2_4190020000(){

   TString inputfile("../4190020000.ntup.root");
  
   TFile* sample = new TFile(inputfile);
   TTree* tree1 = (TTree*)sample->Get("pipi_ntuple");
   TTree* tree2 = (TTree*)sample->Get("pi_ntuple");

   TFile* outputFile = new TFile("4190020000_Validation.root","RECREATE");

   plot_Mrecoil_pipi(sample,tree1,outputFile);
   plot_Mrecoil_pi(sample,tree2,outputFile);

   outputFile->Close();     

}
