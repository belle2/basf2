/*
<header>
<input>../2411460000.ntup.root</input>
<output>2411460000_Validation.root</output>
<contact>Sen Jia; jiasen@buaa.edu.cn</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_2411460000.C
// Check the MM2, cos(theta), M_{J/psi} and M_{psi(2S)} distributions for
// e+e- -> pi+pi-psi(2S) via ISR
// Constributor: Sen Jia
// May 27, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"


void plot_pipipsi(TFile* pfile, TTree* ptree, TFile *outputFile){

  TString pidCuts("vpho_psi2S_Jpsi_mu0_muonID>0.1||vpho_psi2S_Jpsi_mu1_muonID>0.1");

  TH1F* h_mm2 = new TH1F("h_mm2","MM^{2}(#pi#pi#psi(2S))",100,-4,4);
  ptree->Project("h_mm2", "vpho_m2Recoil", pidCuts);
  h_mm2->GetXaxis()->SetTitle("MM^{2}_{#pi#pi#psi(2S)} (GeV/c^{2})");
  h_mm2->GetYaxis()->SetTitle("Events / (0.08 GeV^{2}/c^{4})");
  h_mm2->GetListOfFunctions()->Add(new TNamed("Description", "the square of the mass recoiling against the #pi#pi#psi(2S) system"));
  h_mm2->GetListOfFunctions()->Add(new TNamed("Contact" , "jiasen@buaa.edu.cn"));
  h_mm2->GetListOfFunctions()->Add(new TNamed("Check", "MM^{2}_{#pi#pi#psi(2S)} is expected to peak at 0 GeV^{2}/c^{4}"));

  TH1F* h_cos_theta = new TH1F("h_cos_theta","cos#theta",100,-1,1);
  ptree->Project("h_cos_theta", "vpho_useCMSFrame__bocosTheta__bc", pidCuts);
  h_cos_theta->GetXaxis()->SetTitle("cos#theta");
  h_cos_theta->GetYaxis()->SetTitle("Events / 0.02"); 
  h_cos_theta->GetListOfFunctions()->Add(new TNamed("Description", "the polar-angle distribution of the #pi#pi#psi(2S) system in the e^{+}e^{-} center-of-mass frame"));
  h_cos_theta->GetListOfFunctions()->Add(new TNamed("Contact" , "jiasen@buaa.edu.cn"));
  h_cos_theta->GetListOfFunctions()->Add(new TNamed("Check", "cos#theta is expected to gather around -1 or 1"));

  TH1F* h_jpsi = new TH1F("h_M_jpsi","M(#mu#mu)",100,2.9,3.3);
  ptree->Project("h_M_jpsi", "vpho_psi2S_Jpsi_daughterInvariantMass__bo0__cm1__bc", pidCuts);
  h_jpsi->GetXaxis()->SetTitle("M_{#mu#mu} (GeV/c^{2})");
  h_jpsi->GetYaxis()->SetTitle("Events / 4 MeV");
  h_jpsi->GetListOfFunctions()->Add(new TNamed("Description", "the invariant mass spectra of #mu#mu"));
  h_jpsi->GetListOfFunctions()->Add(new TNamed("Contact" , "jiasen@buaa.edu.cn"));  
  h_jpsi->GetListOfFunctions()->Add(new TNamed("Check", "M_{#mu#mu} is expected to peak at 3.097 GeV/c^{2}"));

  TH1F* h_psi = new TH1F("h_M_psi","M_{#pi#piJ/#psi}",100,3.64,3.74);
  ptree->Project("h_M_psi", "vpho_psi2S_daughterInvariantMass__bo0__cm1__cm2__bc", pidCuts);
  h_psi->GetXaxis()->SetTitle("M_{#pi#piJ/#psi} (GeV/c^{2})");
  h_psi->GetYaxis()->SetTitle("Events / 1 MeV");
  h_psi->GetListOfFunctions()->Add(new TNamed("Description", "the invariant mass spectra of #pi#piJ/#psi"));
  h_psi->GetListOfFunctions()->Add(new TNamed("Contact" , "jiasen@buaa.edu.cn"));  
  h_psi->GetListOfFunctions()->Add(new TNamed("Check", "M_{#pi#piJ/#psi} is expected to peak at 3.686 GeV/c^{2}"));

  outputFile->cd();

  h_mm2->Write();
  h_cos_theta->Write();
  h_jpsi->Write();
  h_psi->Write();
}

void test2_2411460000(){

  TString inputfile("../2411460000.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("h1");

  TFile* outputFile = new TFile("2411460000_Validation.root","RECREATE");

  plot_pipipsi(sample, tree, outputFile);
  outputFile->Close();

}
