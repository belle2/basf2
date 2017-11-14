
/*
<header>
<input>../1263700001.ntup.root</input>
<output>1263700001_Validation.root</output>
<contact> Racha Cheaib, rcheaib@olemiss.edu, Mario Merola, mario.merola@na.infn.it</contact>
<interval>release</interval>
</header>
*/
////////////////////////////////////////////////////////////
//
// test2_1163000003.C
// Check 
//
// Constributor: Racha Cheaib
// Nov 2, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

void plotUpsHad(TFile* pfile, TTree* ptree, TFile *outputFile){

  gStyle->SetOptStat(0);
  gStyle->SetHistMinimumZero();
  
  // General Info
  TH1F* h_Mbc = new TH1F("h_Mbc","Btag Mbc",100,5.22,5.29);
  ptree->Project("h_Mbc", "Upsilon4S_d0_Mbc");
  TH1F* h_DeltaE = new TH1F("h_DeltaE","Btag DeltaE",100,-0.50,0.50);
  ptree->Project("h_DeltaE", "Upsilon4S_d0_deltaE");
  TH1F* h_EExtra = new TH1F("h_EExtra","EExtra",50,0,5);
  ptree->Project("h_EExtra", "Upsilon4S_ROE_eextraSel");
  TH1F* h_SigProb = new TH1F("h_SigProb","B-tag signal probability",50,0,1);
  ptree->Project("h_SigProb", "Upsilon4S_B0_sigProb");

  // Continuum suppression variables
  TH1F* h_R2 = new TH1F("h_R2","R2 (continuum suppression variable)",50,0,1);
  ptree->Project("h_R2", "Upsilon4S_B0_R2");
  TH1F* h_cosTBTO = new TH1F("h_cosTBTO","costheta thrust",20,0,1);
  ptree->Project("h_cosTBTO", "Upsilon4S_B0_cosTBTO");

  // Missing quantities and signal side relevant variables
  TH1F* h_missM2 = new TH1F("h_missM2","squared missing mass",40,0,40);
  //ptree->Project("h_missM2", "Upsilon4S_missM2__boROEclusters__cm0__bc");
  TH1F* h_missP = new TH1F("h_missP","missing momentum",40,0,4);
  //ptree->Project("h_missP", "Upsilon4S_missP__boROEclusters__cm0__bc");
  TH1F* h_DstarMomentum = new TH1F("h_DstarMomentum","D* momentum",40,0,4);
  ptree->Project("h_DstarMomentum", "Upsilon4S_B0_d0_pCMS");
  TH1F* h_Dmass = new TH1F("h_Dmass","invariant mass of D meson from D* decay",40,1.8,1.9);
  ptree->Project("h_Dmass", "Upsilon4S_B0_d0_d0_M");
  TH1F* h_DstarMass = new TH1F("h_DstarMass","invariant mass of D* meson from D* decay",40,1.94,2.04);
  ptree->Project("h_DstarMass", "Upsilon4S_B0_d0_M");


  outputFile->cd();
  TDirectory *dstarlnuHadDir =  outputFile->mkdir("UpsBpDstarlnuHad");
  
  dstarlnuHadDir->cd();


  h_Mbc->Write();
  h_DeltaE->Write();
  h_EExtra->Write();
  h_SigProb->Write();

  h_R2->Write();
  h_cosTBTO->Write();
  h_missM2->Write();
  h_missP->Write();
  h_DstarMomentum->Write();
  h_Dmass->Write();
  h_DstarMass->Write();

}

void plotUpsSL(TFile* pfile, TTree* ptree, TFile *outputFile){


  gStyle->SetOptStat(0);
  gStyle->SetHistMinimumZero();

  TH1F* h_Mbc = new TH1F("h_Mbc","Mbc",100,5.22,5.29);
  ptree->Project("h_Mbc", "Upsilon4S_d0_Mbc");
  TH1F* h_DeltaE = new TH1F("h_DeltaE","DeltaE",100,-0.50,0.50);
  ptree->Project("h_DeltaE", "Upsilon4S_d0_deltaE");
  TH1F* h_EExtra = new TH1F("h_EExtra","EExtra",50,0,5);
  ptree->Project("h_EExtra", "Upsilon4S_ROE_eextraSel");
  TH1F* h_SigProb = new TH1F("h_SigProb","B-tag signal probability",50,0,1);
  ptree->Project("h_SigProb", "Upsilon4S_B0_sigProb");
  TH1F * h_cosThetaBtag= new TH1F("h_cosThetaBtag","Btag cos Theta between particle and true B", 100,-10,5);
  //ptree->Project("h_cosThetaBtag","Upsilon4S_B0_cosThetaBetweenParticleAndTrueB");


  // Continuum suppression variables
  TH1F* h_R2 = new TH1F("h_R2","R2 (continuum suppression variable)",50,0,1);
  ptree->Project("h_R2", "Upsilon4S_B0_R2"); //Change to R2EventLevel
  TH1F* h_cosTBTO = new TH1F("h_cosTBTO","costheta thrust",20,0,1);
  ptree->Project("h_cosTBTO", "Upsilon4S_B0_cosTBTO");

  // Missing quantities and signal side relevant variables
  TH1F* h_missM2 = new TH1F("h_missM2","squared missing mass",40,0,40);
 // ptree->Project("h_missM2", "Upsilon4S_missM2__boROEclusters__cm0__bc");
  TH1F* h_missP = new TH1F("h_missP","missing momentum",40,0,4);
  //ptree->Project("h_missP", "Upsilon4S_missP__boROEclusters__cm0__bc");
  TH1F* h_DstarMomentum = new TH1F("h_DstarMomentum","D* momentum",40,0,4);
  ptree->Project("h_DstarMomentum", "Upsilon4S_B0_d0_pCMS");
  TH1F* h_Dmass = new TH1F("h_Dmass","invariant mass of D meson from D* decay",40,1.8,1.9);
  ptree->Project("h_Dmass", "Upsilon4S_B0_d0_d0_M");
  TH1F* h_DstarMass = new TH1F("h_DstarMass","invariant mass of D* meson from D* decay",40,1.94,2.04);
  ptree->Project("h_DstarMass", "Upsilon4S_B0_d0_M");

  outputFile->cd();
  TDirectory *dstarlnuSLDir =  outputFile->mkdir("UpsBpDstarlnuSLTag");
  
  dstarlnuSLDir->cd();


  h_Mbc->Write();
  h_DeltaE->Write();
  h_EExtra->Write();
  h_SigProb->Write();
  h_cosThetaBtag->Write();
  h_R2->Write();
  h_cosTBTO->Write();
  h_DstarMomentum->Write();
  h_DstarMass->Write();
  h_Dmass->Write();

}


void test2_1263700001(){

  TString inputfile("1263700001.ntuple.root");
l
  TFile* sample = new TFile(inputfile);
  TTree* treeUpsHad = (TTree*)sample->Get("UpsBplusHad");
  TTree* treeUpsSL = (TTree*)sample->Get("UpsBplusSl");
  

  TFile* outputFile = new TFile("1263700001_Validation.root","RECREATE");
  
  TString Cuts= "";
  plotUpsHad(sample, treeUpsHad, outputFile );
  plotUpsSL(sample, treeUpsSL, outputFile);

  outputFile->Close();

}
