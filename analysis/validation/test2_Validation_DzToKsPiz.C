/*
<header>
<input>../DzToKsPiz_MDSTtoNTUP.root</input>
<output>DzToKsPiz_Validation.root</output>
<contact>Giulia Casarosa; giulia.casarosa@pi.infn.it</contact>
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

/* Validation script to determine benchmarks for PID cuts. */

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

  h_sigmat_all = new TH1F("h_sigmat_all", "full momentum range", 100,0, 0.02);
  ptree->Project("h_sigmat_all", "D0_K_S0_FTE",tmCuts);

  h_sig_all = new TH1F("h_sig_all", "full momentum range", 100,0, 200);
  ptree->Project("h_sig_all", "D0_K_S0_FT/D0_K_S0_FTE",tmCuts);
  
  
  outputFile->cd();
  
  h_tres_all->Write();
  h_sigmat_all->Write();
  h_sig_all->Write();

}


void test2_Validation_DzToKsPiz(){

  TString inputfile("../DzToKsPiz_MDSTtoNTUP.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("dz");

  TFile* outputFile = new TFile("DzToKsPiz_Validation.root","RECREATE");

  plotStd(sample, tree, outputFile);

  plotTime(sample, tree, outputFile);


  outputFile->Close();

}
