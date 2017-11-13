/*
<header>
<input>../1290310000.ntup.root</input>
<output>1290310000_Validation.root</output>
<contact>Mario Merola; mario.merola@na.infn.it</contact>
<interval>release</interval>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_1290310000.C
// Check 
//
// Constributor: Mario Merola
// May 26, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

void plot4StauHad(TFile* pfile, TTree* ptree, TFile *outputFile, TString Cuts, TString TauMode){

  gStyle->SetOptStat(0);
  gStyle->SetHistMinimumZero();
  
  // General Info
  TH1F* h_Mbc = new TH1F("h_Mbc","Mbc",100,5.22,5.29);
  ptree->Project("h_Mbc", "Upsilon4S_d0_Mbc", Cuts);
  TH1F* h_DeltaE = new TH1F("h_DeltaE","DeltaE",100,-0.50,0.50);
  ptree->Project("h_DeltaE", "Upsilon4S_d0_deltaE", Cuts);
  TH1F* h_EExtra = new TH1F("h_EExtra","EExtra",50,0,5);
  ptree->Project("h_EExtra", "Upsilon4S_ROE_eextraSel", Cuts);
  TH1F* h_SigProb = new TH1F("h_SigProb","B-tag signal probability",50,0,1);
  ptree->Project("h_SigProb", "Upsilon4S_B_sigProb", Cuts);

  // Continuum suppression variables
  TH1F* h_R2 = new TH1F("h_R2","R2 (continuum suppression variable)",50,0,1);
  ptree->Project("h_R2", "Upsilon4S_B_R2", Cuts);
  TH1F* h_cosTBTO = new TH1F("h_cosTBTO","costheta thrust",20,0,1);
  ptree->Project("h_cosTBTO", "Upsilon4S_B_cosTBTO", Cuts);

  // Missing quantities and signal side relevant variables
  TH1F* h_missM2 = new TH1F("h_missM2","squared missing mass",40,0,40);
  ptree->Project("h_missM2", "Upsilon4S_missM2__boROEclusters__cm0__bc", Cuts);
  TH1F* h_missP = new TH1F("h_missP","missing momentum",40,0,4);
  ptree->Project("h_missP", "Upsilon4S_missP__boROEclusters__cm0__bc", Cuts);
  TH1F* h_missCosth = new TH1F("h_missCosth","missing momentum costheta",40,-1,1);
  ptree->Project("h_missCosth", "TMath::Cos(Upsilon4S_missPTheta__boROEclusters__cm0__bc)", Cuts);
  TH1F* h_PSig = new TH1F("h_PSig","signal side momentum",40,0,4);
  ptree->Project("h_PSig", "Upsilon4S_B_d0_pCMS", Cuts);
  TH1F* h_pi0mass = new TH1F("h_pi0mass","invariant mass of pi0 from tau decay",30,0.105,0.165);
  ptree->Project("h_pi0mass", "Upsilon4S_B1_d0_d1_M", Cuts);



  outputFile->cd();
  TDirectory *tauHadDir =  outputFile->mkdir("HadTag_" + TauMode + "Tau");
  
  tauHadDir->cd();


  h_Mbc->Write();
  h_DeltaE->Write();
  h_EExtra->Write();
  h_SigProb->Write();

  h_R2->Write();
  h_cosTBTO->Write();
  h_missM2->Write();
  h_missP->Write();
  h_missCosth->Write();
  h_PSig->Write();
  h_pi0mass->Write();


}

void plot4StauSL(TFile* pfile, TTree* ptree, TFile *outputFile, TString Cuts, TString TauMode){



  gStyle->SetOptStat(0);
  gStyle->SetHistMinimumZero();

  TH1F* h_Mbc = new TH1F("h_Mbc","Mbc",100,5.22,5.29);
  ptree->Project("h_Mbc", "Upsilon4S_d0_Mbc", Cuts);
  TH1F* h_DeltaE = new TH1F("h_DeltaE","DeltaE",100,-0.50,0.50);
  ptree->Project("h_DeltaE", "Upsilon4S_d0_deltaE", Cuts);
  TH1F* h_EExtra = new TH1F("h_EExtra","EExtra",50,0,5);
  ptree->Project("h_EExtra", "Upsilon4S_ROE_eextraSel", Cuts);
  TH1F* h_SigProb = new TH1F("h_SigProb","B-tag signal probability",50,0,1);
  ptree->Project("h_SigProb", "Upsilon4S_B_sigProb", Cuts);


  outputFile->cd();
  TDirectory *tauSLDir =  outputFile->mkdir("SLTag_" + TauMode + "Tau");
  
  tauSLDir->cd();


  h_Mbc->Write();
  h_DeltaE->Write();
  h_EExtra->Write();
  h_SigProb->Write();

}


void test2_1290310000(){

  TString inputfile("../1290310000.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree4StauHad = (TTree*)sample->Get("BHadBtaunu");
  TTree* tree4StauSL = (TTree*)sample->Get("BSLBtaunu");

  TFile* outputFile = new TFile("1290310000_Validation.root","RECREATE");
  
  TString Cuts = "(Upsilon4S_B1_d0_dmID==4 && Upsilon4S_B_tau_eid>0.750) || (Upsilon4S_B1_d0_dmID==3 && Upsilon4S_B_tau_muid>0.625 && Upsilon4S_B_tau_eid<0.750) || (((Upsilon4S_B1_d0_dmID==5 && iCand==0) || (Upsilon4S_B1_d0_dmID==6 && nCands>1 && iCand==0)) && Upsilon4S_B_tau_piid>0.429 && Upsilon4S_B_tau_eid<0.750 && Upsilon4S_B_tau_muid<0.625)";
  
  TString CutsLepChannels = "(Upsilon4S_B1_d0_dmID==4 && Upsilon4S_B_tau_eid>0.750) || (Upsilon4S_B1_d0_dmID==3 && Upsilon4S_B_tau_muid>0.625 && Upsilon4S_B_tau_eid<0.750)";

  TString CutsMuChannel = "(Upsilon4S_B1_d0_dmID==3 && Upsilon4S_B_tau_muid>0.625 && Upsilon4S_B_tau_eid<0.750)";

  TString CutsEleChannel = "(Upsilon4S_B1_d0_dmID==4 && Upsilon4S_B_tau_eid>0.750)";
  
  TString CutsHadChannels = "(((Upsilon4S_B1_d0_dmID==5 && iCand==0) || (Upsilon4S_B1_d0_dmID==6 && nCands>1 && iCand==0)) && Upsilon4S_B_tau_piid>0.429 && Upsilon4S_B_tau_eid<0.750 && Upsilon4S_B_tau_muid<0.625)";

  TString CutsPiChannel = "((Upsilon4S_B1_d0_dmID==5 && iCand==0) && Upsilon4S_B_tau_piid>0.429 && Upsilon4S_B_tau_eid<0.750 && Upsilon4S_B_tau_muid<0.625)";

  TString CutsPiPi0Channel = "((Upsilon4S_B1_d0_dmID==6 && nCands>1 && iCand==0) && Upsilon4S_B_tau_piid>0.429 && Upsilon4S_B_tau_eid<0.750 && Upsilon4S_B_tau_muid<0.625)";
  
  TString TauMode = "All";
  TString TauModeLep = "Lep";
  TString TauModeMu = "Mu";
  TString TauModeEle = "Ele";
  TString TauModeHad = "Had";
  TString TauModePi = "Pi";
  TString TauModePiPi0 = "PiPi0";

  plot4StauHad(sample, tree4StauHad, outputFile, Cuts, TauMode );
  plot4StauHad(sample, tree4StauHad, outputFile, CutsLepChannels,TauModeLep );
  plot4StauHad(sample, tree4StauHad, outputFile, CutsMuChannel,TauModeMu );
  plot4StauHad(sample, tree4StauHad, outputFile, CutsEleChannel,TauModeEle );
  plot4StauHad(sample, tree4StauHad, outputFile, CutsHadChannels,TauModeHad );
  plot4StauHad(sample, tree4StauHad, outputFile, CutsPiChannel,TauModePi );
  plot4StauHad(sample, tree4StauHad, outputFile, CutsPiPi0Channel,TauModePiPi0 );
  plot4StauSL(sample, tree4StauSL, outputFile, Cuts, TauMode);

  outputFile->Close();

}
