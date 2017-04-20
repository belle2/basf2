/*
<header>
<input>../DstToDzPip_DzToKmPip.root</input>
<output>DstToDzPip_DzToKmPip_Validation.root</output>
<contact>Giulia Casarosa; giulia.casarosa@pi.infn.it</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// Validation_DstToDzPip_DzToKmPip,C
// Check the proper time and the slow pion
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

  TString tmCuts("(DST_isSignal == 1)");

  TH1F* hp = new TH1F("h_p","D0 momentum",100,0,6);
  ptree->Project("h_p", "DST_D0_P", tmCuts);

  outputFile->cd();

  hp->Write();

}


void plotTime(TFile* pfile, TTree* ptree, TFile *outputFile){
 
  // define some cuts
  TString tmCuts("(DST_isSignal == 1)");
  
  Double_t ax = 0.005;
 
  h_tres_all = new TH1F("h_tres_all", "full momentum range", 100,-ax, ax);
  ptree->Project("h_tres_all", "DST_D0_FT - DST_D0_MCFT",tmCuts);
  
  
  outputFile->cd();
  
  h_tres_all->Write();

}

void plotTrack(TFile* pfile, TTree* ptree, TFile *outputFile){
 
  // define some cuts
  TString tmCuts("(DST_isSignal == 1)");
  
  
  h_pis_pVal = new TH1F("h_pis_pVal", "pVal slow pion", 100, 0 , 1 );
  ptree->Project("h_pis_pVal", "DST_pi_TrPval",tmCuts);

  h_pi_pVal = new TH1F("h_pi_pVal", "pVal D0 pion", 100, 0 , 1 );
  ptree->Project("h_pi_pVal", "DST_D0_pi_TrPval",tmCuts);
  h_k_pVal = new TH1F("h_k_pVal", "pVal D0 kaon", 100, 0 , 1 );
  ptree->Project("h_k_pVal", "DST_D0_K_TrPval",tmCuts);
  
  
  outputFile->cd();
  
  h_pis_pVal->Write();
  h_pi_pVal->Write();
  h_k_pVal->Write();

}

void test2_Validation_DstToDzPip_DzToKmPip(){

  TString inputfile("../DstToDzPip_DzToKmPip.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("dst");

  TFile* outputFile = new TFile("DstToDzPip_DzToKmPip_Validation.root","RECREATE");

  plotStd(sample, tree, outputFile);

  plotTime(sample, tree, outputFile);

  plotTrack(sample, tree, outputFile);

  outputFile->Close();

}
