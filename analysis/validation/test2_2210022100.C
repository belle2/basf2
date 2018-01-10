/*
<header>
<input>../2210022100.ntup.root</input>
<output>2210022100_Validation.root</output>
<contact>Yeqi Chen; chenyq15@mail.ustc.edu.cn</contact>
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

  TH1F* h_D0_p = new TH1F("h_D0_p","D0 momentum",30,0,7);
  ptree->Project("h_D0_p", "D0_P", tmCuts);
  h_D0_p->GetListOfFunctions()->Add(new TNamed("Description", "D0 -> KSpi0, Momentum of D0"));
  h_D0_p->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_D0_p->GetListOfFunctions()->Add(new TNamed("Check", "Momentum of D0 vary from 0 - 7.0 GeV/c^{2}"));

  TH1F* h_ks_p = new TH1F("h_ks_p","Ks momentum",30,0,6);
  ptree->Project("h_ks_p", "D0_K_S0_P", tmCuts);
  h_ks_p->GetListOfFunctions()->Add(new TNamed("Description", "D0 -> KSpi0, Momentum of KS"));
  h_ks_p->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_ks_p->GetListOfFunctions()->Add(new TNamed("Check", "Momentum of D0 vary from 0 - 6.0 GeV/c^{2}"));

  TH1F* h_piz_p = new TH1F("h_piz_p","pi0 momentum",30,0,5);
  ptree->Project("h_piz_p", "D0_pi0_P", tmCuts);
  h_piz_p->GetListOfFunctions()->Add(new TNamed("Description", "D0 -> KSpi0, Momentum of pi0"));
  h_piz_p->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_piz_p->GetListOfFunctions()->Add(new TNamed("Check", "Momentum of D0 vary from 0 - 5.0 GeV/c^{2}"));

  outputFile->cd();

  h_D0_p->Write();
  h_ks_p->Write();
  h_piz_p->Write();

}


void plotTime(TFile* pfile, TTree* ptree, TFile *outputFile){
 
  // define some cuts
  TString tmCuts("(D0_isSignal == 1)");
  Double_t ax = 0.01;
 
  h_tres_all = new TH1F("h_tres_all", "D0 flight time resolution", 30,-ax, ax);
  ptree->Project("h_tres_all", "D0_K_S0_FT - D0_K_S0_MCFT",tmCuts);
  h_tres_all->GetXaxis()->SetTitle("t_{reco} - t_{gen} (ns)");
  h_tres_all->GetListOfFunctions()->Add(new TNamed("Description", "D0 -> KSpi0, D0 flight time resolution"));
  h_tres_all->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_tres_all->GetListOfFunctions()->Add(new TNamed("Check", "D0 time resolution about 140 fs"));

  h_sigmat_all = new TH1F("h_sigmat_all", "D0 flight time error", 20,0, 0.01);
  ptree->Project("h_sigmat_all", "D0_K_S0_FTE",tmCuts);
  h_sigmat_all->GetXaxis()->SetTitle("#sigma_{t} (ns)");
  h_sigmat_all->GetListOfFunctions()->Add(new TNamed("Description", "D0 -> KSpi0, D0 flight time error"));
  h_sigmat_all->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_sigmat_all->GetListOfFunctions()->Add(new TNamed("Check", "D0 flight time error is about 73 fs"));

  h_sig_all = new TH1F("h_sig_all", "D0 flight time significance", 20,0, 200);
  ptree->Project("h_sig_all", "D0_K_S0_FT/D0_K_S0_FTE",tmCuts);
  h_sig_all->GetListOfFunctions()->Add(new TNamed("Description", "D0 -> KSpi0,  D0 flight time significance"));
  h_sig_all->GetListOfFunctions()->Add(new TNamed("Contact" , "chenyq15@mail.ustc.edu.cn"));
  h_sig_all->GetListOfFunctions()->Add(new TNamed("Check", "Ratio of D0 flight time resolution to time error"));
  
  
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
