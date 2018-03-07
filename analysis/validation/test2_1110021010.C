/*
<header>
<input>../1110021010.ntup.root</input>
<output>1110021010_Validation.root</output>
<contact>Saurabh Sandilya; saurabhsandilya@gmail.com</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_1110021010.C
// Check the validation distributions for
//  B0->rho0[pi+pi-]gamma
// Constributor: Saurabh Sandilya
//               Sam Cunliffe (Oct 2017)
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

void plot_validplots(TFile* pfile, TTree* ptree, TFile *outputFile){
  char const *contactForAllPlots = "Saurabh Sandilya (saurabhsandilya@gmail.com)";
  char const *title = "B^{0}#rightarrow#rho^{0}#gamma";

  TH1F* h_mbc = new TH1F("h_mbc",title,180,5.2,5.29);
  ptree->Project("h_mbc", "B0_mbc", "1==1");
  h_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");
  h_mbc->GetListOfFunctions()->Add(new TNamed("Description", "Beam constrained invariant mass distrubtion of B0 -> rho0 gamma"));
  h_mbc->GetListOfFunctions()->Add(new TNamed("Check", "Distribution should be centred at B^{0} mass. Tail at low mass"));
  h_mbc->GetListOfFunctions()->Add(new TNamed("Contact", contactForAllPlots));

  TH1F* h_deltae = new TH1F("h_deltae",title,100,-0.5,0.5);
  ptree->Project("h_deltae", "B0_deltae", "1==1");
  h_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)");
  h_deltae->GetListOfFunctions()->Add(new TNamed("Description", "#Delta E distribution from B^{0}#rightarrow #rho^{0}#gamma"));
  h_deltae->GetListOfFunctions()->Add(new TNamed("Check", "Distribution should be centred at zero, tail to LHS."));
  h_deltae->GetListOfFunctions()->Add(new TNamed("Contact", contactForAllPlots));


  TH1F* h_egam = new TH1F("h_egam",title,50,1.5,4.0);
  ptree->Project("h_egam", "B0_gamma_P", "1==1");
  h_egam->GetXaxis()->SetTitle("E_{lab}(#gamma) (GeV)");
  h_egam->GetListOfFunctions()->Add(new TNamed("Description", "Lab frame photon energy distribution from B0 -> rho0 gamma"));
  h_egam->GetListOfFunctions()->Add(new TNamed("Check", "Broad distribution bewteen 1.5 and 3 GeV."));
  h_egam->GetListOfFunctions()->Add(new TNamed("Contact", contactForAllPlots));
  
  TH1F* h_eparentgam = new TH1F("h_eparentgam",title,100,1.5,4.0); 
  ptree->Project("h_eparentgam", "B0_Egamma", "1==1");
  h_eparentgam->GetXaxis()->SetTitle("E(#gamma) (GeV)");
  h_eparentgam->GetListOfFunctions()->Add(new TNamed("Description", "Parent rest frame photon energy distribution from B0 -> rho0 gamma"));
  h_eparentgam->GetListOfFunctions()->Add(new TNamed("Check", "Sharp distribution at 2.5 GeV."));
  h_eparentgam->GetListOfFunctions()->Add(new TNamed("Contact", contactForAllPlots));
  


  outputFile->cd();

  h_mbc->Write();
  h_deltae->Write();
  h_egam->Write();
  h_eparentgam->Write();
}

void test2_1110021010(){

  TString inputfile("../1110021010.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("h1");

  TFile* outputFile = new TFile("1110021010_Validation.root","RECREATE");

  plot_validplots(sample, tree, outputFile);
  outputFile->Close();

}
