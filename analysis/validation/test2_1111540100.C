/*
 <header>
 <input>../1111540100.ntup.root</input>
 <output>1111540100_Validation.root</output>
 <contact>A. Mordà; morda@pd.infn.it</contact>
 </header>
 */

//////////////////////////////////////////////////////////
//
// test2_1111540100.C
// Check the Delta E and Mbc distributions for
//  B0->J/psi(ee)K0S(pi+pi-)
// Constributor: A. Mordà
// May 25, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

/* Validation script to determine benchmarks for PID cuts. */

void plot_dembc(TFile* pfile, TTree* ptree, TFile *outputFile){

 TString tmCuts("(B0_Jpsi_mcPDG==443&&B0_K_S0_mcPDG==310)");

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
                              

                           
 outputFile->cd();

 h_mbc->Write();
 h_deltae->Write();
}

void test2_1111540100(){

 TString inputfile("../1111540100.ntup.root");

 TFile* sample = new TFile(inputfile);
 TTree* tree = (TTree*)sample->Get("B0");
 TFile* outputFile = new TFile("../1111540100_Validation.root","RECREATE");

 plot_dembc(sample, tree, outputFile);
 outputFile->Close();

}
