/*
<header>
<input>../DstToDzPip_DzToKmPip_MDSTtoNTUP.root</input>
<output>DstToDzPip_DzToKmPip_Validation.root</output>
<contact>Giulia Casarosa; giulia.casarosa@pi.infn.it</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// Validation_DstToDzPip_DzToKmPip.C
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

  TH1F* h_D0_p = new TH1F("h_D0_p","D0 momentum",100,0,7);
  ptree->Project("h_D0_p", "DST_D0_P", tmCuts);

  TH1F* h_pis_p = new TH1F("h_pis_p","soft pi momentum",100,0,0.5);
  ptree->Project("h_pis_p", "DST_pi_P", tmCuts);
  TH1F* h_pi_p = new TH1F("h_pi_p","pi momentum",100,0,5);
  ptree->Project("h_pi_p", "DST_D0_pi_P", tmCuts);
  TH1F* h_k_p = new TH1F("h_k_p","K momentum",100,0,5);
  ptree->Project("h_k_p", "DST_D0_K_P", tmCuts);

  outputFile->cd();

  h_D0_p->Write();
  h_pis_p->Write();
  h_pi_p->Write();
  h_k_p->Write();

}


void plotTime(TFile* pfile, TTree* ptree, TFile *outputFile){
 
  // define some cuts
  TString tmCuts("(DST_isSignal == 1)");
  TString nHits("DST_D0_pi_nPXDHits > 0 ||DST_D0_K_nPXDHits > 0 ");
  TString cuts(nHits+"&&"+tmCuts);
  Double_t ax = 1;
 
  h_tres_all = new TH1F("h_tres_all", "full momentum range", 100,-ax, ax);
  ptree->Project("h_tres_all", "(DST_D0_FT - DST_D0_MCFT)*1000",cuts);

  h_sigmat_all = new TH1F("h_sigmat_all", "full momentum range", 100,0, 0.5);
  ptree->Project("h_sigmat_all", "DST_D0_FTE*1000",cuts);

  h_sig_all = new TH1F("h_sig_all", "full momentum range", 100,0, 20);
  ptree->Project("h_sig_all", "DST_D0_FT/DST_D0_FTE",tmCuts);
  
  
  outputFile->cd();
  
  h_tres_all->Write();
  h_sigmat_all->Write();
  h_sig_all->Write();

}

void plotTrack(TFile* pfile, TTree* ptree, TFile *outputFile){
 
  // define some cuts
  TString tmCuts("(DST_isSignal == 1)");
  TString nHits("DST_D0_pi_nPXDHits > 0 ||DST_D0_K_nPXDHits > 0 ");
  TString cuts(nHits+"&&"+tmCuts);

  h_pis_d0 = new TH2F("h_pis_d0VSpt","d0 error vs transverse momentum",100,0,0.5,100,0,0.1);
  ptree->Project("h_pis_d0VSpt","DST_pi_d0err:sqrt(DST_pi_P4[0]*DST_pi_P4[0] + DST_pi_P4[1]*DST_pi_P4[1])",cuts);
  TProfile * pfx_pis_d0 = h_pis_d0->ProfileX("hprof_pis_d0");

  h_pi_d0 = new TH2F("h_pi_d0VSpt","d0 error vs transverse momentum",100,0,4,100,0,0.1);
  ptree->Project("h_pi_d0VSpt","DST_D0_pi_d0err:sqrt(DST_D0_pi_P4[0]*DST_D0_pi_P4[0] + DST_D0_pi_P4[1]*DST_D0_pi_P4[1])",cuts);
  TProfile * pfx_pi_d0 = h_pi_d0->ProfileX("hprof_pi_d0");

  h_k_d0 = new TH2F("h_k_d0VSpt","d0 error vs transverse momentum",100,0,4,100,0,0.1);
  ptree->Project("h_k_d0VSpt","DST_D0_K_d0err:sqrt(DST_D0_K_P4[0]*DST_D0_K_P4[0] + DST_D0_K_P4[1]*DST_D0_K_P4[1])",cuts);
  TProfile * pfx_k_d0 = h_k_d0->ProfileX("hprof_k_d0");


  h_pis_z0 = new TH2F("h_pis_z0VSpt","z0 error vs transverse momentum",100,0,0.5,100,0,0.1);
  ptree->Project("h_pis_z0VSpt","DST_pi_z0err:sqrt(DST_pi_P4[0]*DST_pi_P4[0] + DST_pi_P4[1]*DST_pi_P4[1])",cuts);
  TProfile * pfx_pis_z0 = h_pis_z0->ProfileX("hprof_pis_z0");

  h_pi_z0 = new TH2F("h_pi_z0VSpt","z0 error vs transverse momentum",100,0,4,100,0,0.1);
  ptree->Project("h_pi_z0VSpt","DST_D0_pi_z0err:sqrt(DST_D0_pi_P4[0]*DST_D0_pi_P4[0] + DST_D0_pi_P4[1]*DST_D0_pi_P4[1])",cuts);
  TProfile * pfx_pi_z0 = h_pi_z0->ProfileX("hprof_pi_z0");

  h_k_z0 = new TH2F("h_k_z0VSpt","z0 error vs transverse momentum",100,0,4,100,0,0.1);
  ptree->Project("h_k_z0VSpt","DST_D0_K_z0err:sqrt(DST_D0_K_P4[0]*DST_D0_K_P4[0] + DST_D0_K_P4[1]*DST_D0_K_P4[1])",cuts);
  TProfile * pfx_k_z0 = h_k_z0->ProfileX("hprof_k_z0");

  h_pis_pVal = new TH1F("h_pis_pVal", "pVal slow pion", 100, 0 , 1 );
  ptree->Project("h_pis_pVal", "DST_pi_TrPval",cuts);

  h_pi_pVal = new TH1F("h_pi_pVal", "pVal D0 pion", 100, 0 , 1 );
  ptree->Project("h_pi_pVal", "DST_D0_pi_TrPval",cuts);
  h_k_pVal = new TH1F("h_k_pVal", "pVal D0 kaon", 100, 0 , 1 );
  ptree->Project("h_k_pVal", "DST_D0_K_TrPval",cuts);
  
  
  outputFile->cd();

  //  h_pis_d0->Write();
  pfx_pis_d0->Write();
  //  h_pi_d0->Write();
  pfx_pi_d0->Write();
  //  h_k_d0->Write();
  pfx_k_d0->Write();

  //  h_pis_z0->Write();
  pfx_pis_z0->Write();
  //  h_pi_z0->Write();
  pfx_pi_z0->Write();
  //  h_k_z0->Write();
  pfx_k_z0->Write();

  h_pis_pVal->Write();
  h_pi_pVal->Write();
  h_k_pVal->Write();

}

void test2_Validation_DstToDzPip_DzToKmPip(){

  TString inputfile("../DstToDzPip_DzToKmPip_MDSTtoNTUP.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("dst");

  TFile* outputFile = new TFile("DstToDzPip_DzToKmPip_Validation.root","RECREATE");

  plotStd(sample, tree, outputFile);

  plotTime(sample, tree, outputFile);

  plotTrack(sample, tree, outputFile);

  outputFile->Close();

}
