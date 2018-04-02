/*
<header>
<input>../1110021001.ntup.root</input>
<output>1110021001_Validation.root</output>
<contact>Saurabh Sandilya; saurabhsandilya@gmail.com</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_1110021001.C
// Check the Delta E and Mbc distributions for
//  B0->K*0[K+pi-]gamma
// Contributors: Saurabh Sandilya
//               Sam Cunliffe (Last Update Oct 27, 2017)
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

void plot_validplots(TFile* pfile, TTree* ptree, TFile *outputFile){
  char const *contactForAllPlots = "S. Cunliffe (sam.cunliffe@desy.de), S. Sandilya (saurabhsandilya@gmail.com), M. Schram (malachi.schram@desy.de)";
  char const *title = "B^{0}#rightarrowK*^{0}#gamma";

  TH1F* h_mbc = new TH1F("h_mbc",title,180,5.2,5.29);
  ptree->Project("h_mbc", "B0_mbc", "1==1");
  h_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");
  h_mbc->GetListOfFunctions()->Add(new TNamed("Description", "Beam constrained invariant mass distrubtion of B0 -> K*0 gamma"));
  h_mbc->GetListOfFunctions()->Add(new TNamed("Check", "Distribution should be centred at B^{0} mass. Tail at low mass"));
  h_mbc->GetListOfFunctions()->Add(new TNamed("Contact", contactForAllPlots));
  h_mbc->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.7, pvalue-error=0.05"))

  TH1F* h_mkpi = new TH1F("h_mkpi",title,60,0.6,1.2);
  ptree->Project("h_mkpi", "B0_KST0_M", "1==1");
  h_mkpi->GetXaxis()->SetTitle("m_{K#pi} (GeV/c^{2})");
  h_mkpi->GetListOfFunctions()->Add(new TNamed("Description", "K#pi invariant mass distrubtion of K*(892)^{0} from B0 -> K*0 gamma"));
  h_mkpi->GetListOfFunctions()->Add(new TNamed("Check", "Distribution should be a Breit-Wigner shape centred at 0.896 GeV/c^{2}. "));
  h_mkpi->GetListOfFunctions()->Add(new TNamed("Contact", contactForAllPlots));
  h_mkpi->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.7, pvalue-error=0.05"))


  TH1F* h_deltae = new TH1F("h_deltae",title,100,-0.5,0.5);
  ptree->Project("h_deltae", "B0_deltae", "1==1");
  h_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)");
  h_deltae->GetListOfFunctions()->Add(new TNamed("Description", "#Delta E distribution from the decay B0 -> K*0 gamma"));
  h_deltae->GetListOfFunctions()->Add(new TNamed("Check", "Distribution should be centred at zero, tail to LHS."));
  h_deltae->GetListOfFunctions()->Add(new TNamed("Contact", contactForAllPlots));
  h_deltae->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.7, pvalue-error=0.05"))
  
  TH1F* h_egam = new TH1F("h_egam",title,50,1.5,4.0);
  ptree->Project("h_egam", "B0_gamma_P", "1==1");
  h_egam->GetXaxis()->SetTitle("E_{lab}(#gamma) (GeV)");
  h_egam->GetListOfFunctions()->Add(new TNamed("Description", "Lab frame photon energy distribution from the decay B0 -> K*0 gamma"));
  h_egam->GetListOfFunctions()->Add(new TNamed("Check", "Broad distribution bewteen 1.5 and 3 GeV."));
  h_egam->GetListOfFunctions()->Add(new TNamed("Contact", contactForAllPlots));
  h_egam->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.7, pvalue-error=0.05"))

  TH1F* h_eparentgam = new TH1F("h_eparentgam",title,100,1.5,4.0); ptree->Project("h_eparentgam", "B0_Egamma", "1==1");
  h_eparentgam->GetXaxis()->SetTitle("E(#gamma) (GeV)");
  h_eparentgam->GetListOfFunctions()->Add(new TNamed("Description", "Parent rest frame photon energy distribution from the decay B0 -> K*0 gamma"));
  h_eparentgam->GetListOfFunctions()->Add(new TNamed("Check", "Sharp distribution at 2.5 GeV."));
  h_eparentgam->GetListOfFunctions()->Add(new TNamed("Contact", contactForAllPlots));
  h_eparentgam->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.7, pvalue-error=0.05"))


  outputFile->cd();

  h_mbc->Write();
  h_mkpi->Write();
  h_deltae->Write();
  h_egam->Write();
  h_eparentgam->Write();
}

void test2_1110021001(){

  TString inputfile("../1110021001.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("h1");

  TFile* outputFile = new TFile("1110021001_Validation.root","RECREATE");

  plot_validplots(sample, tree, outputFile);
  outputFile->Close();

}
