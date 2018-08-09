/*
<header>
<input>../1163300003.ntup.root</input>
<output>1163300003_Validation.root</output>
<contact> Racha Cheaib, rcheaib@olemiss.edu, Mario Merola, mario.merola@na.infn.it</contact>
<interval>nightly</interval>
</header>
*/
////////////////////////////////////////////////////////////
//
// test2_1163000003.C
//
// Constributor: Racha Cheaib
// Nov 2, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

const char *contact = "rcheaib@olemiss.edu, mario.merola@na.infn.it"; 

void plotUpsHad(TFile* pfile, TTree* ptree, TFile *outputFile){

  gStyle->SetOptStat(0);
  gStyle->SetHistMinimumZero();

  const char *title = "B#rightarrow D^{*}#tau#nu (Hadronic tag)";
  
  // General Info
  TH1F* h_Mbc = new TH1F("h_had_Mbc",title,100,5.22,5.29);
  ptree->Project("h_had_Mbc", "Upsilon4S_d0_Mbc");
  h_Mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");
  h_Mbc->GetListOfFunctions()->Add(new TNamed("Description", "Beam constrained mass"));
  h_Mbc->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_Mbc->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_DeltaE = new TH1F("h_had_DeltaE",title,100,-0.50,0.50);
  ptree->Project("h_had_DeltaE", "Upsilon4S_d0_deltaE");
  h_DeltaE->GetXaxis()->SetTitle("#Delta E (GeV)");
  h_DeltaE->GetListOfFunctions()->Add(new TNamed("Description", "Peaks at zero, longer tail to low energy"));
  h_DeltaE->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_DeltaE->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_EExtra = new TH1F("h_had_EExtra",title,50,0,5);
  ptree->Project("h_had_EExtra", "Upsilon4S_ROE_eextraSel");
  h_EExtra->GetXaxis()->SetTitle("E_{extra} (GeV)");
  h_EExtra->GetListOfFunctions()->Add(new TNamed("Description", "Extra energy in the event"));
  h_EExtra->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_EExtra->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_SigProb = new TH1F("h_had_SigProb",title,50,0,1);
  ptree->Project("h_had_SigProb", "Upsilon4S_B0_sigProb");
  h_SigProb->GetXaxis()->SetTitle("signal probability");
  h_SigProb->GetListOfFunctions()->Add(new TNamed("Description", "B-tag signal probability"));
  h_SigProb->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_SigProb->GetListOfFunctions()->Add(new TNamed("Contact", contact));

  // Continuum suppression variables
  TH1F* h_R2 = new TH1F("h_had_R2",title,50,0,1);
  ptree->Project("h_had_R2", "Upsilon4S_B0_R2");
  h_R2->GetXaxis()->SetTitle("R2 (continuum suppression variable)");
  h_R2->GetListOfFunctions()->Add(new TNamed("Description", "The continuum suppression variable, R2"));
  h_R2->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_R2->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_cosTBTO = new TH1F("h_had_cosTBTO",title,20,0,1);
  ptree->Project("h_had_cosTBTO", "Upsilon4S_B0_cosTBTO");
  h_cosTBTO->GetXaxis()->SetTitle("cos(#theta_{thrust})");
  h_cosTBTO->GetListOfFunctions()->Add(new TNamed("Description", "Cosine of the angle between the B and the thrust axis of the event"));
  h_cosTBTO->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_cosTBTO->GetListOfFunctions()->Add(new TNamed("Contact", contact));

  // Missing quantities and signal side relevant variables
  TH1F* h_missM2 = new TH1F("h_had_missM2",title,40,0,40);
  ptree->Project("h_had_missM2", "Upsilon4S_missM2__boROEclusters__cm0__bc");
  h_missM2->GetXaxis()->SetTitle("squared missing mass M_{miss}^{2} (GeV^{2}/c^{4})");
  h_missM2->GetListOfFunctions()->Add(new TNamed("Description", "The squared missing mass"));
  h_missM2->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_missM2->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_missP = new TH1F("h_had_missP",title,40,0,4);
  ptree->Project("h_had_missP", "Upsilon4S_missP__boROEclusters__cm0__bc");
  h_missP->GetXaxis()->SetTitle("missing momentum p_{miss} (GeV/c)");
  h_missP->GetListOfFunctions()->Add(new TNamed("Description", "The missing momentum"));
  h_missP->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_missP->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_DstarMomentum = new TH1F("h_had_DstarMomentum",title,40,0,4);
  ptree->Project("h_had_DstarMomentum", "Upsilon4S_B0_d0_pCMS");
  h_DstarMomentum->GetXaxis()->SetTitle("p_{D*} (GeV/c)");
  h_DstarMomentum->GetListOfFunctions()->Add(new TNamed("Description", "The momentum of the D^{*}"));
  h_DstarMomentum->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_DstarMomentum->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_Dmass = new TH1F("h_had_Dmass",title,40,1.8,1.9);
  ptree->Project("h_had_Dmass", "Upsilon4S_B0_d0_d0_M");
  h_Dmass->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_Dmass->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D meson from D* decay"));
  h_Dmass->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_Dmass->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_DstarMass = new TH1F("h_had_DstarMass","",40,1.94,2.04);
  ptree->Project("h_had_DstarMass", "Upsilon4S_B0_d0_M");
  h_DstarMass->GetXaxis()->SetTitle("m_{D*} (GeV/c^{2})");
  h_DstarMass->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D* meson from B decay"));
  h_DstarMass->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_DstarMass->GetListOfFunctions()->Add(new TNamed("Contact", contact));


  outputFile->cd();


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

  const char *title = "B#rightarrow D^{*}l#nu (Semi-leptonic tag)";

  TH1F* h_Mbc = new TH1F("h_sl_Mbc",title,100,5.22,5.29);
  ptree->Project("h_sl_Mbc", "Upsilon4S_d0_Mbc");
  h_Mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");
  h_Mbc->GetListOfFunctions()->Add(new TNamed("Description", "Beam constrained mass"));
  h_Mbc->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_Mbc->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_DeltaE = new TH1F("h_sl_DeltaE",title,100,-0.50,0.50);
  ptree->Project("h_sl_DeltaE", "Upsilon4S_d0_deltaE");
  h_DeltaE->GetXaxis()->SetTitle("#Delta E (GeV)");
  h_DeltaE->GetListOfFunctions()->Add(new TNamed("Description", "Peaks at zero, longer tail to low energy"));
  h_DeltaE->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_DeltaE->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_EExtra = new TH1F("h_sl_EExtra",title,50,0,5);
  ptree->Project("h_sl_EExtra", "Upsilon4S_ROE_eextraSel");
  h_EExtra->GetXaxis()->SetTitle("E_{extra} (GeV)");
  h_EExtra->GetListOfFunctions()->Add(new TNamed("Description", "Extra energy in the event"));
  h_EExtra->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_EExtra->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_SigProb = new TH1F("h_sl_SigProb",title,50,0,1);
  ptree->Project("h_sl_SigProb", "Upsilon4S_B0_sigProb");
  h_SigProb->GetXaxis()->SetTitle("signal probability");
  h_SigProb->GetListOfFunctions()->Add(new TNamed("Description", "B-tag signal probability"));
  h_SigProb->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_SigProb->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F * h_cosThetaBtag= new TH1F("h_sl_cosThetaBtag",title, 100,-10,5);
  ptree->Project("h_sl_cosThetaBtag","Upsilon4S_B0_cosThetaBetweenParticleAndTrueB");
  h_cosThetaBtag->GetXaxis()->SetTitle("cos(#theta_{Btag})");
  h_cosThetaBtag->GetListOfFunctions()->Add(new TNamed("Description", "Btag cos Theta between particle and true B"));
  h_cosThetaBtag->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_cosThetaBtag->GetListOfFunctions()->Add(new TNamed("Contact", contact));



  // Continuum suppression variables
  TH1F* h_R2 = new TH1F("h_sl_R2",title,50,0,1);
  ptree->Project("h_sl_R2", "Upsilon4S_B0_R2");
  h_R2->GetXaxis()->SetTitle("R2 (continuum suppression variable)");
  h_R2->GetListOfFunctions()->Add(new TNamed("Description", "The continuum suppression variable, R2"));
  h_R2->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_R2->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_cosTBTO = new TH1F("h_sl_cosTBTO",title,20,0,1);
  ptree->Project("h_sl_cosTBTO", "Upsilon4S_B0_cosTBTO");
  h_cosTBTO->GetXaxis()->SetTitle("cos(#theta_{thrust})");
  h_cosTBTO->GetListOfFunctions()->Add(new TNamed("Description", "Cosine of the angle between the B and the thrust axis of the event"));
  h_cosTBTO->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_cosTBTO->GetListOfFunctions()->Add(new TNamed("Contact", contact));

  // Missing quantities and signal side relevant variables
  TH1F* h_missM2 = new TH1F("h_sl_missM2",title,40,0,40);
  ptree->Project("h_sl_missM2", "Upsilon4S_missM2__boROEclusters__cm0__bc");
  h_missM2->GetXaxis()->SetTitle("squared missing mass M_{miss}^{2} (GeV^{2}/c^{4})");
  h_missM2->GetListOfFunctions()->Add(new TNamed("Description", "The squared missing mass"));
  h_missM2->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_missM2->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_missP = new TH1F("h_sl_missP",title,40,0,4);
  ptree->Project("h_sl_missP", "Upsilon4S_missP__boROEclusters__cm0__bc");
  h_missP->GetXaxis()->SetTitle("missing momentum p_{miss} (GeV/c)");
  h_missP->GetListOfFunctions()->Add(new TNamed("Description", "The missing momentum"));
  h_missP->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_missP->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_DstarMomentum = new TH1F("h_sl_DstarMomentum",title,40,0,4);
  ptree->Project("h_sl_DstarMomentum", "Upsilon4S_B0_d0_pCMS");
  h_DstarMomentum->GetXaxis()->SetTitle("p_{D*} (GeV/c)");
  h_DstarMomentum->GetListOfFunctions()->Add(new TNamed("Description", "The momentum of the D^{*}"));
  h_DstarMomentum->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_DstarMomentum->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_Dmass = new TH1F("h_sl_Dmass",title,40,1.8,1.9);
  ptree->Project("h_sl_Dmass", "Upsilon4S_B0_d0_d0_M");
  h_Dmass->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_Dmass->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D meson from D* decay"));
  h_Dmass->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_Dmass->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  TH1F* h_DstarMass = new TH1F("h_sl_DstarMass","",40,1.94,2.04);
  ptree->Project("h_sl_DstarMass", "Upsilon4S_B0_d0_M");
  h_DstarMass->GetXaxis()->SetTitle("m_{D*} (GeV/c^{2})");
  h_DstarMass->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D* meson from B decay"));
  h_DstarMass->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_DstarMass->GetListOfFunctions()->Add(new TNamed("Contact", contact));

  outputFile->cd();


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
  h_missM2->Write();
  h_missP->Write();

}


void test2_1163300003(){

  TString inputfile("../1163300003.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* treeUpsHad = (TTree*)sample->Get("UpsB0Had");
  TTree* treeUpsSL = (TTree*)sample->Get("UpsB0sl");
  

  TFile* outputFile = new TFile("1163300003_Validation.root","RECREATE");
  
  TString Cuts= "";
  plotUpsHad(sample, treeUpsHad, outputFile );
  plotUpsSL(sample, treeUpsSL, outputFile);

  outputFile->Close();

}
