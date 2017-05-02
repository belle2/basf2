/*
<header>
<input>../DpToPipPiz_MDSTtoNTUP.root</input>
<output>DpToPipPiz_Validation.root</output>
<contact>Giulia Casarosa; giulia.casarosa@pi.infn.it</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// Validation_DpToPipPiz.C
// Check 
//
// Constributor: Giulia Casarosa
// May 2, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

/* Validation script to determine benchmarks for PID cuts. */

void plotStd(TFile* pfile, TTree* ptree, TFile *outputFile){

  TString tmCuts("(D_isSignal == 1)");

  TH1F* h_D_p = new TH1F("h_D_p","D+ momentum",100,0,7);
  ptree->Project("h_D_p", "D_P", tmCuts);

  TH1F* h_pip_p = new TH1F("h_pip_p","pi+ momentum",100,0,6);
  ptree->Project("h_pip_p", "D_pi_P", tmCuts);
  TH1F* h_piz_p = new TH1F("h_piz_p","pi0 momentum",100,0,5);
  ptree->Project("h_piz_p", "D_pi0_P", tmCuts);

  outputFile->cd();

  h_D_p->Write();
  h_pip_p->Write();
  h_piz_p->Write();

}

void plotTrack(TFile* pfile, TTree* ptree, TFile *outputFile){
 
  // define some cuts
  TString tmCuts("(D_isSignal == 1)");
  TString nHits("D_pi_nPXDHits > 0");
  TString cuts(nHits+"&&"+tmCuts);

  h_pi_d0 = new TH2F("h_pi_d0VSpt","d0 error vs transverse momentum",100,0,4,100,0,0.1);
  ptree->Project("h_pi_d0VSpt","D_pi_d0err:sqrt(D_pi_P4[0]*D_pi_P4[0] + D_pi_P4[1]*D_pi_P4[1])",cuts);
  TProfile * pfx_pi_d0 = h_pi_d0->ProfileX("hprof_pis_d0");

  h_pi_z0 = new TH2F("h_pi_z0VSpt","z0 error vs transverse momentum",100,0,4,100,0,0.1);
  ptree->Project("h_pi_z0VSpt","D_pi_z0err:sqrt(D_pi_P4[0]*D_pi_P4[0] + D_pi_P4[1]*D_pi_P4[1])",cuts);
  TProfile * pfx_pi_z0 = h_pi_z0->ProfileX("hprof_pi_z0");

  h_pi_pVal = new TH1F("h_pi_pVal", "pVal pion", 100, 0 , 1 );
  ptree->Project("h_pi_pVal", "D_pi_TrPval",cuts);
  
  
  outputFile->cd();

  //  h_pi_d0->Write();
  pfx_pi_d0->Write();

  //  h_pi_z0->Write();
  pfx_pi_z0->Write();

  h_pi_pVal->Write();

}

void test2_Validation_DpToPipPiz(){

  TString inputfile("../DpToPipPiz_MDSTtoNTUP.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("dp");

  TFile* outputFile = new TFile("DpToPipPiz_Validation.root","RECREATE");

  plotStd(sample, tree, outputFile);

  plotTrack(sample, tree, outputFile);

  outputFile->Close();

}
