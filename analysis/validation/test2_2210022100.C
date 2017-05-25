/*
<header>
<input>../2210022100.ntup.root</input>
<output>2210022100_Validation.root</output>
<contact>Jake Bennett; jvbennett@cmu.edu</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// Validation_DzToKsPiz.C
// Check the Ks flight significance
//
// Constributor: Giulia Casarosa
// April 20, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

void plotStd(TFile* pfile, TTree* ptree, TFile *outputFile){

  TString tmCuts("(D0_isSignal == 1)");

  TH1F* h_D0_p = new TH1F("h_D0_p","D0 momentum",100,0,7);
  ptree->Project("h_D0_p", "D0_P", tmCuts);

  TH1F* h_ks_p = new TH1F("h_ks_p","Ks momentum",100,0,6);
  ptree->Project("h_ks_p", "D0_K_S0_P", tmCuts);
  TH1F* h_piz_p = new TH1F("h_piz_p","pi0 momentum",100,0,5);
  ptree->Project("h_piz_p", "D0_pi0_P", tmCuts);

  outputFile->cd();

  h_D0_p->Write();
  h_ks_p->Write();
  h_piz_p->Write();

}


void plotTime(TFile* pfile, TTree* ptree, TFile *outputFile){
 
  // define some cuts
  TString tmCuts("(D0_isSignal == 1)");
  Double_t ax = 0.01;
 
  h_tres_all = new TH1F("h_tres_all", "full momentum range", 100,-ax, ax);
  ptree->Project("h_tres_all", "D0_K_S0_FT - D0_K_S0_MCFT",tmCuts);
  h_tres_all->GetXaxis()->SetTitle("t_{reco} - t_{gen} (ns)");

  h_sigmat_all = new TH1F("h_sigmat_all", "full momentum range", 100,0, 0.01);
  ptree->Project("h_sigmat_all", "D0_K_S0_FTE",tmCuts);
  h_sigmat_all->GetXaxis()->SetTitle("#sigma_{t} (ns)");

  h_sig_all = new TH1F("h_sig_all", "full momentum range", 100,0, 200);
  ptree->Project("h_sig_all", "D0_K_S0_FT/D0_K_S0_FTE",tmCuts);
  
  
  outputFile->cd();
  
  h_tres_all->Write();
  h_sigmat_all->Write();
  h_sig_all->Write();

}


void test2_2210022100(){

  TString inputfile("../2210022100.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("dz");

  TFile* outputFile = new TFile("2210022100_Validation.root","RECREATE");

  plotStd(sample, tree, outputFile);

  plotTime(sample, tree, outputFile);


  outputFile->Close();

}
