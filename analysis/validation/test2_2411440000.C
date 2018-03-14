/*
<header>
<input>../2411440000.ntup.root</input>
<output>2411440000_Validation.root</output>
<contact>Yubo Li; liyb@pku.edu.cn</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_2411440000.C
// Check the MM2, M_{J/psi} and M_{pipiJ/psi} distributions for
//  e+e- -> pi+pi-J/psi[mu+mu-]   
// Constributor: Yubo Li
// May 27, 2017
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"


void plot_pipijpsi(TFile* pfile, TTree* ptree, TFile *outputFile){

  TString Cuts("vpho_Jpsi_mu0_muonID > 0.1 && vpho_Jpsi_mu1_muonID >0.1");
  TH1F* h_pipijpsi = new TH1F("h_pipijpsi","M_{#pi#piJ/#psi}", 50, 6.0, 12.0);
  ptree->Project("h_pipijpsi", "vpho_daughterInvariantMass__bo0__cm1__cm2__bc", Cuts);
  h_pipijpsi->GetXaxis()->SetTitle("M_{#pi#piJ/#psi} (GeV/c^{2})");
  h_pipijpsi->GetYaxis()->SetTitle("Events / (40 MeV/c^{2})");
  h_pipijpsi->GetListOfFunctions()->Add(new TNamed("Description", "e^{+} e^{-} #rightarrow #pi^{+} #pi^{-} J/#psi(#mu#mu)  M_{#pi#piJ/#psi} (GeV/c^{2})"));
  h_pipijpsi->GetListOfFunctions()->Add(new TNamed("Contact" , "liyb@pku.edu.cn"));
  h_pipijpsi->GetListOfFunctions()->Add(new TNamed("Check", "M_{#pi#piJ/#psi} distribution should be smooth and has more events in higher mass side"));

  TH1F* h_mm2 = new TH1F("h_mm2","MM^{2}_{#pi#piJ/#psi}",100,-1,1);
  ptree->Project("h_mm2", "vpho_m2Recoil", Cuts);
  h_mm2->GetXaxis()->SetTitle("MM^{2}_{#pi#piJ/#psi} (GeV^{2}/c^{4})");
  h_mm2->GetYaxis()->SetTitle("Events / (0.02 GeV^{2}/c^{4})");
  h_mm2->GetListOfFunctions()->Add(new TNamed("Description", "e^{+} e^{-} #rightarrow #pi^{+} #pi^{-} J/#psi(#mu#mu) MM^{2}_{#pi#piJ/#psi} (GeV^{2}/c^{4}) "));  
  h_mm2->GetListOfFunctions()->Add(new TNamed("Contact" , "liyb@pku.edu.cn"));
  h_mm2->GetListOfFunctions()->Add(new TNamed("Check", "MM^{2}_{#pi#piJ/#psi} is expected to peak at 0 GeV^{2}/c^{4}"));

  TH1F* h_jpsi = new TH1F("h_jpsi","M_{#mu#mu}",100,2.9,3.2);
  ptree->Project("h_jpsi", "vpho_Jpsi_daughterInvariantMass__bo0__cm1__bc", Cuts);
  h_jpsi->GetXaxis()->SetTitle("M_{#mu#mu} (GeV/c^{2})");
  h_jpsi->GetYaxis()->SetTitle("Events / (3 MeV/c^{2})");
  h_jpsi->GetListOfFunctions()->Add(new TNamed("Description", "e^{+} e^{-} #rightarrow #pi^{+} #pi^{-} J/#psi(#mu#mu) M_{#mu#mu} MeV/c^{2}")); 
  h_jpsi->GetListOfFunctions()->Add(new TNamed("Contact" , "liyb@pku.edu.cn"));
  h_jpsi->GetListOfFunctions()->Add(new TNamed("Check", "M_{#mu#mu} is expected to peak at 3.1 GeV/c^{2}"));
  
  outputFile->cd();

  h_pipijpsi->Write();
  h_jpsi->Write();
  h_mm2->Write();
}

void test2_2411440000(){

  TString inputfile("../2411440000.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("h1");

  TFile* outputFile = new TFile("2411440000_Validation.root","RECREATE");

  plot_pipijpsi(sample, tree, outputFile);
  outputFile->Close();

}
