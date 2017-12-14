/*
<header>
<input>../2610030000.ntup.root</input>
<output>2610030000_Validation.root</output>
<contact>Yeqi Chen; chenyq15@mail.ustc.edu.cn</contact>
</header>
*/

////////////////////////////////////////////////////////////
// D*+ -> D0(Kpi)pi_s
// test2_2610030000.C
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

  TH1F* h_Dst_Q = new TH1F("h_Dst_Q","Q Value",30,3, 9);
  ptree->Project("h_Dst_Q", "DST_Q*1000", tmCuts);
  h_Dst_Q->GetXaxis()->SetTitle("Q-calue (MeV)");
  h_Dst_Q->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, released energy of Dst decay"));
  h_Dst_Q->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_Dst_Q->GetListOfFunctions()->Add(new TNamed("Check", "Q value expected to peak at 5.85  MeV/c^{2}"));

  TH1F* h_D0_p = new TH1F("h_D0_p","D0 momentum",20,0,7);
  ptree->Project("h_D0_p", "DST_D0_P", tmCuts);
  h_D0_p->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, Momentum of D0"));
  h_D0_p->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_D0_p->GetListOfFunctions()->Add(new TNamed("Check", "Momentum of D0 should vary from 0 - 7.0 GeV/c^{2}"));

  TH1F* h_pis_p = new TH1F("h_pis_p","soft pi momentum",20,0,0.6);
  ptree->Project("h_pis_p", "DST_pi_P", tmCuts);
  h_pis_p->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, Momentum of slow pion"));
  h_pis_p->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_pis_p->GetListOfFunctions()->Add(new TNamed("Check", "Momentum of slow pion is smaller than 0.6 GeV/c^{2}"));

  TH1F* h_pi_p = new TH1F("h_pi_p","pi momentum",20,0,5);
  ptree->Project("h_pi_p", "DST_D0_pi_P", tmCuts);
  h_pi_p->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, Momentum of pion"));
  h_pi_p->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_pi_p->GetListOfFunctions()->Add(new TNamed("Check", "Momentum of pion vary from 0 - 5 GeV/c^{2}"));

  TH1F* h_k_p = new TH1F("h_k_p","K momentum",20,0,5);
  ptree->Project("h_k_p", "DST_D0_K_P", tmCuts);
  h_k_p->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, Momentum of Kaon"));
  h_k_p->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_k_p->GetListOfFunctions()->Add(new TNamed("Check", "Momentum of kaon vary from 0 - 5 GeV/c^{2}"));

  outputFile->cd();

  h_Dst_Q->Write();
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
 
  h_tres_all = new TH1F("h_tres_all", "D0 flight time resolution", 25,-ax, ax);
  ptree->Project("h_tres_all", "(DST_D0_FT - DST_D0_MCFT)*1000",cuts);
  h_tres_all->GetXaxis()->SetTitle("t_{reco} - t_{gen} (ps)");
  h_tres_all->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, hist->GetTitle()"));
  h_tres_all->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_tres_all->GetListOfFunctions()->Add(new TNamed("Check", "D0 time resolution about 140 fs"));

  h_sigmat_all = new TH1F("h_sigmat_all", "D0 flight time error", 25,0, 0.5);
  ptree->Project("h_sigmat_all", "DST_D0_FTE*1000",cuts);
  h_sigmat_all->GetXaxis()->SetTitle("#sigma_{t} (ps)");
  h_sigmat_all->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, hist->GetTitle()"));
  h_sigmat_all->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_sigmat_all->GetListOfFunctions()->Add(new TNamed("Check", "D0 flight time error is about 73 fs"));

  h_sig_all = new TH1F("h_sig_all", "D0 flight time significance", 25,0, 20);
  ptree->Project("h_sig_all", "DST_D0_FT/DST_D0_FTE",tmCuts);
  h_sig_all->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, hist->GetTitle()"));
  h_sig_all->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_sig_all->GetListOfFunctions()->Add(new TNamed("Check", "Ratio of D0 flight time resolution to time error"));
  
  
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

  h_pis_d0 = new TH2F("h_pis_d0VSpt","d0 error vs transverse momentum",10,0,0.5,10,0,0.1);
  ptree->Project("h_pis_d0VSpt","DST_pi_d0err:sqrt(DST_pi_P4[0]*DST_pi_P4[0] + DST_pi_P4[1]*DST_pi_P4[1])",cuts);
  TProfile * pfx_pis_d0 = h_pis_d0->ProfileX("hprof_pis_d0");
  pfx_pis_d0->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, hist->GetTitle()"));
  pfx_pis_d0->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  pfx_pis_d0->GetListOfFunctions()->Add(new TNamed("Check", "ProjectionX of d0 error and transverse momentum for slow pion"));

  h_pi_d0 = new TH2F("h_pi_d0VSpt","d0 error vs transverse momentum",10,0,4,10,0,0.1);
  ptree->Project("h_pi_d0VSpt","DST_D0_pi_d0err:sqrt(DST_D0_pi_P4[0]*DST_D0_pi_P4[0] + DST_D0_pi_P4[1]*DST_D0_pi_P4[1])",cuts);
  TProfile * pfx_pi_d0 = h_pi_d0->ProfileX("hprof_pi_d0");
  pfx_pi_d0->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, hist->GetTitle()"));
  pfx_pi_d0->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  pfx_pi_d0->GetListOfFunctions()->Add(new TNamed("Check", "ProjectionX of d0 error and transverse momentum for pion"));

  h_k_d0 = new TH2F("h_k_d0VSpt","d0 error vs transverse momentum",10,0,4,10,0,0.1);
  ptree->Project("h_k_d0VSpt","DST_D0_K_d0err:sqrt(DST_D0_K_P4[0]*DST_D0_K_P4[0] + DST_D0_K_P4[1]*DST_D0_K_P4[1])",cuts);
  TProfile * pfx_k_d0 = h_k_d0->ProfileX("hprof_k_d0");
  pfx_k_d0->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, hist->GetTitle()"));
  pfx_k_d0->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  pfx_k_d0->GetListOfFunctions()->Add(new TNamed("Check", "ProjectionX of d0 error and transverse momentum for kaon"));


  h_pis_z0 = new TH2F("h_pis_z0VSpt","z0 error vs transverse momentum",10,0,0.5,10,0,0.1);
  ptree->Project("h_pis_z0VSpt","DST_pi_z0err:sqrt(DST_pi_P4[0]*DST_pi_P4[0] + DST_pi_P4[1]*DST_pi_P4[1])",cuts);
  TProfile * pfx_pis_z0 = h_pis_z0->ProfileX("hprof_pis_z0");
  pfx_pis_z0->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, hist->GetTitle()"));
  pfx_pis_z0->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  pfx_pis_z0->GetListOfFunctions()->Add(new TNamed("Check", "ProjectionX of z0 error and transverse momentum for slow pion"));

  h_pi_z0 = new TH2F("h_pi_z0VSpt","z0 error vs transverse momentum",10,0,4,10,0,0.1);
  ptree->Project("h_pi_z0VSpt","DST_D0_pi_z0err:sqrt(DST_D0_pi_P4[0]*DST_D0_pi_P4[0] + DST_D0_pi_P4[1]*DST_D0_pi_P4[1])",cuts);
  TProfile * pfx_pi_z0 = h_pi_z0->ProfileX("hprof_pi_z0");
  pfx_pi_z0->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, hist->GetTitle()"));
  pfx_pi_z0->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  pfx_pi_z0->GetListOfFunctions()->Add(new TNamed("Check", "ProjectionX of z0 error and transverse momentum for pion"));

  h_k_z0 = new TH2F("h_k_z0VSpt","z0 error vs transverse momentum",10,0,4,10,0,0.1);
  ptree->Project("h_k_z0VSpt","DST_D0_K_z0err:sqrt(DST_D0_K_P4[0]*DST_D0_K_P4[0] + DST_D0_K_P4[1]*DST_D0_K_P4[1])",cuts);
  TProfile * pfx_k_z0 = h_k_z0->ProfileX("hprof_k_z0");
  pfx_k_z0->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, hist->GetTitle()"));
  pfx_k_z0->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  pfx_k_z0->GetListOfFunctions()->Add(new TNamed("Check", "ProjectionX of z0 error and transverse momentum for kaon"));

  h_pis_pVal = new TH1F("h_pis_pVal", "pVal slow pion", 20, 0 , 1 );
  ptree->Project("h_pis_pVal", "DST_pi_TrPval",cuts);
  h_pis_pVal->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, hist->GetTitle()"));
  h_pis_pVal->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_pis_pVal->GetListOfFunctions()->Add(new TNamed("Check", "pVal slow pion, vary from 0 - 1"));

  h_pi_pVal = new TH1F("h_pi_pVal", "pVal D0 pion", 20, 0 , 1 );
  ptree->Project("h_pi_pVal", "DST_D0_pi_TrPval",cuts);
  h_pi_pVal->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, hist->GetTitle()"));
  h_pi_pVal->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_pi_pVal->GetListOfFunctions()->Add(new TNamed("Check", "pVal pion, vary from 0 - 1"));

  h_k_pVal = new TH1F("h_k_pVal", "pVal D0 kaon", 20, 0 , 1 );
  ptree->Project("h_k_pVal", "DST_D0_K_TrPval",cuts);
  h_k_pVal->GetListOfFunctions()->Add(new TNamed("Description", "D*+ -> D0(Kpi)pi_s, hist->GetTitle()"));
  h_k_pVal->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_k_pVal->GetListOfFunctions()->Add(new TNamed("Check", "pVal kaon, vary from 0 - 1"));
  
  
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

void test2_2610030000(){

  TString inputfile("../2610030000.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("dst");

  TFile* outputFile = new TFile("2610030000_Validation.root","RECREATE");

  plotStd(sample, tree, outputFile);

  plotTime(sample, tree, outputFile);

  plotTrack(sample, tree, outputFile);

  outputFile->Close();

}
