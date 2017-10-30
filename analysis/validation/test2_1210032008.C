/*
<header>
<input>../1210032008.ntup.root</input>
<output>1210032008_Validation.root</output>
<contact>Komarov Ilya; ilya.komarov@desy.de</contact>
<description>Distribuions used in angular analysis for 
transverse-polarised B->K*(->K+pi0)rho(->pi+pi-) candidates:
  <ul>
      <li>&Delta;E</li>
      <li>Mbc</li>
      <li>M(&rho;(770))</li>
      <li>M(K*(892))</li>
      <li>cos(&theta;(&pi;&pi;))</li>
      <li>cos(&theta;(K&pi;))</li>
  </ul>
For each variables three histograms are created:
  <ul>
      <li>Gen-level historgram, i.e. distirbuion of the variable
       for denerated candidate</li>
      <li>True value historgram, i.e. distirbuion of true (generated)
       values of the given variable for the correctly reconstructed candidates</li>
      <li>Rec-level historgram, i.e. distirbuion of reconstructed
       values of the given variable for the correctly reconstructed candidates</li>
  </ul>
</description>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_1210032008.C
// Check Mbc, DeltaE, MR, MK, helK, helR
//
// Constributor: Komarov Ilya
// October 30, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

/* Validation script for angular analysis variables. */

void plot(TTree* ptree, TTree* pmctree, TFile *outputFile){


  TString sigCut("(B_isSignal == 1)");

  TH1F* gen_mbc = new TH1F("gen_mbc","M_{bc} (gen-level)",100,5.25,5.29);
  pmctree->Project("gen_mbc", "B_Mbc");
  gen_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");

  TH1F* true_mbc = new TH1F("true_mbc","True M_{bc} of reconstructed signals",100,5.25,5.29);
  ptree->Project("true_mbc", "B_MCT_Mbc", sigCut);
  true_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");

  TH1F* rec_mbc = new TH1F("rec_mbc","M_{bc} of reconstructed signals",100,5.25,5.29);
  ptree->Project("rec_mbc", "B_MCT_Mbc", sigCut);
  rec_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");

  TH1F* gen_deltae = new TH1F("gen_deltae","#DeltaE",100,-0.2,0.15);
  pmctree->Project("gen_deltae", "B_deltaE");
  gen_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)");

  TH1F* true_deltae = new TH1F("true_deltae","#DeltaE",100,-0.2,0.15);
  ptree->Project("true_deltae", "B_MCT_deltaE", sigCut);
  true_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)");

  TH1F* rec_deltae = new TH1F("rec_deltae","#DeltaE",100,-0.2,0.15);
  ptree->Project("rec_deltae", "B_deltaE", sigCut);
  rec_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)");

  TH1F* gen_MK = new TH1F("gen_MK","M_{K*(892)^{+}}",100,0.792,0.992);
  pmctree->Project("gen_MK", "B_MK");
  gen_MK->GetXaxis()->SetTitle("M_{K*(892)^{+}} (GeV/c^{2})");

  TH1F* true_MK = new TH1F("true_MK","M_{K*(892)^{+}}",100,0.792,0.992);
  ptree->Project("true_MK", "B_MCT_MK", sigCut);
  true_MK->GetXaxis()->SetTitle("M_{K*(892)^{+}} (GeV/c^{2})");

  TH1F* rec_MK = new TH1F("rec_MK","M_{K*(892)^{+}}",100,0.792,0.992);
  ptree->Project("rec_MK", "B_MK", sigCut);
  rec_MK->GetXaxis()->SetTitle("M_{K*(892)^{+}} (GeV/c^{2})");

  TH1F* gen_MR = new TH1F("gen_MR","M_{#rho(770)^{0}}",100,0.5,1.05);
  pmctree->Project("gen_MR", "B_MR");
  gen_MR->GetXaxis()->SetTitle("M_{#rho(770)^{0}} (GeV/c^{2})");

  TH1F* true_MR = new TH1F("true_MR","M_{#rho(770)^{0}}",100,0.5,1.05);
  ptree->Project("true_MR", "B_MCT_MR", sigCut);
  true_MR->GetXaxis()->SetTitle("M_{#rho(770)^{0}} (GeV/c^{2})");

  TH1F* rec_MR = new TH1F("rec_MR","M_{#rho(770)^{0}}",100,0.5,1.05);
  ptree->Project("rec_MR", "B_MR", sigCut);
  rec_MR->GetXaxis()->SetTitle("M_{#rho(770)^{0}} (GeV/c^{2})");

  TH1F* gen_helK = new TH1F("gen_helK","cos(#theta_{K^+#pi^0})",100,-1.,1.);
  pmctree->Project("gen_helK", "B_helK");
  gen_helK->GetXaxis()->SetTitle("cos(#theta_{K^+#pi^0})");

  TH1F* true_helK = new TH1F("true_helK","cos(#theta_{K^+#pi^0})",100,-1.,1.);
  ptree->Project("true_helK", "B_MCT_helK", sigCut);
  true_helK->GetXaxis()->SetTitle("cos(#theta_{K^+#pi^0})");

  TH1F* rec_helK = new TH1F("rec_helK","cos(#theta_{K^+#pi^0})",100,-1.,1.);
  ptree->Project("rec_helK", "B_helK", sigCut);
  rec_helK->GetXaxis()->SetTitle("cos(#theta_{K^+#pi^0})");

  TH1F* gen_helR = new TH1F("gen_helR","cos(#theta_{#pi^+#pi^-})",100,-1.,1.);
  pmctree->Project("gen_helR", "B_helR");
  gen_helR->GetXaxis()->SetTitle("cos(#theta_{#pi^+#pi^-})");

  TH1F* true_helR = new TH1F("true_helR","cos(#theta_{#pi^+#pi^-})",100,-1.,1.);
  ptree->Project("true_helR", "B_MCT_helR", sigCut);
  true_helR->GetXaxis()->SetTitle("cos(#theta_{#pi^+#pi^-})");

  TH1F* rec_helR = new TH1F("rec_helR","cos(#theta_{#pi^+#pi^-})",100,-1.,1.);
  ptree->Project("rec_helR", "B_helR", sigCut);
  rec_helR->GetXaxis()->SetTitle("cos(#theta_{#pi^+#pi^-})");

  TNamed* mo_expert_nc = new TNamed("MetaOptions", "expert, nocompare");
  TNamed* mo_nc = new TNamed("MetaOptions", "expert, nocompare");

  gen_mbc->GetListOfFunctions()->Add(mo_expert_nc);
  true_mbc->GetListOfFunctions()->Add(mo_expert_nc);
  rec_mbc->GetListOfFunctions()->Add(mo_expert_nc);
  gen_deltae->GetListOfFunctions()->Add(mo_expert_nc);
  true_deltae->GetListOfFunctions()->Add(mo_expert_nc);
  rec_deltae->GetListOfFunctions()->Add(mo_expert_nc);
  gen_MK->GetListOfFunctions()->Add(mo_expert_nc);
  true_MK->GetListOfFunctions()->Add(mo_expert_nc);
  rec_MK->GetListOfFunctions()->Add(mo_expert_nc);
  gen_MR->GetListOfFunctions()->Add(mo_expert_nc);
  true_MR->GetListOfFunctions()->Add(mo_expert_nc);
  rec_MR->GetListOfFunctions()->Add(mo_expert_nc);
  gen_helK->GetListOfFunctions()->Add(mo_expert_nc);
  true_helK->GetListOfFunctions()->Add(mo_expert_nc);
  rec_helK->GetListOfFunctions()->Add(mo_nc);
  gen_helR->GetListOfFunctions()->Add(mo_expert_nc);
  true_helR->GetListOfFunctions()->Add(mo_expert_nc);
  rec_helR->GetListOfFunctions()->Add(mo_nc);

  outputFile->cd();
  gen_mbc->Write();
  true_mbc->Write();
  rec_mbc->Write();
  gen_deltae->Write();
  true_deltae->Write();
  rec_deltae->Write();
  gen_MK->Write();
  true_MK->Write();
  rec_MK->Write();
  gen_MR->Write();
  true_MR->Write();
  rec_MR->Write();
  gen_helK->Write();
  true_helK->Write();
  rec_helK->Write();
  gen_helR->Write();
  true_helR->Write();
  rec_helR->Write();
}




void test2_1210032008(){

  TString inputfile("../1210032008.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("B2KstRho");
  TTree* mctree = (TTree*)sample->Get("MCB2KstRho");

  TFile* outputFile = new TFile("1210032008_Validation.root","RECREATE");

  plot(tree, mctree, outputFile);


  outputFile->Close();

}
