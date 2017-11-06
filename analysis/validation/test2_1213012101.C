/*
<header>
<input>../1213012101.ntup.root</input>
<output>1213012101_Validation.root</output>
<contact>Minakshi Nayak; minakshi.nayak@wayne.edu</contact>
</header>
*/

////////////////////////////////////////////////////////////
//
// test2_1213012101.C
// Check Mbc, DeltaE, MD, MKs, and Mpi0
//
// Constributor: Minakshi Nayak
// May 17, 2017
// Modified: October 31, 2017
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TApplication.h"
#include "TGraph.h"
#include <TROOT.h>
#include <TSystem.h>

 /* Draw the validation plots. */

void plot(TFile* pfile, TTree* ptree, TFile *outputFile){


  TString tmCuts("(B_isSignal == 1)");

  TH1F* h_mbc = new TH1F("h_mbc","B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{0})K^{+}",100,5.25,5.29);
  //  ptree->Project("h_mbc", "B_mbc", tmCuts);
  ptree->Project("h_mbc", "B_mbc");
  h_mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");
  h_mbc->GetYaxis()->SetTitle("Evts/bin");
  h_mbc->GetListOfFunctions()->Add(new TNamed("Description", "B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{0})K^{+}  M_{bc} [GeV/c^{2}]"));
  h_mbc->GetListOfFunctions()->Add(new TNamed("Contact" , "minakshi.nayak@wayne.edu"));
  h_mbc->GetListOfFunctions()->Add(new TNamed("Check", "mass is expected to peak at 5.279 GeV/c^{2} with resolution around 3 MeV"));

  TH1F* h_deltae = new TH1F("h_deltae","B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{0})K^{+}",100,-0.2,0.15);
  //  ptree->Project("h_deltae", "B_deltae", tmCuts);
  ptree->Project("h_deltae", "B_deltae");
  h_deltae->GetXaxis()->SetTitle("#DeltaE (GeV)");
  h_deltae->GetYaxis()->SetTitle("Evts/bin");
  h_deltae->GetListOfFunctions()->Add(new TNamed("Description", "B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{0})K^{+}  #DeltaE [GeV]"));
  h_deltae->GetListOfFunctions()->Add(new TNamed("Contact" , "minakshi.nayak@wayne.edu"));
  h_deltae->GetListOfFunctions()->Add(new TNamed("Check", "#Delta E is expected to peak at 0 GeV with resolution around 48 MeV"));


  TH1F* h_MKs = new TH1F("h_MKs","B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{0})K^{+}",100,0.45,0.55);
  //  ptree->Project("h_MKs", "B_D0_K_S0_M", tmCuts);
  ptree->Project("h_MKs", "B_D0_K_S0_M");
  h_MKs->GetXaxis()->SetTitle("M_{K_{S}} (GeV/c^{2})");
  h_MKs->GetYaxis()->SetTitle("Evts/bin");
  h_MKs->GetListOfFunctions()->Add(new TNamed("Description", "B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{0})K^{+}  M_{K_{S}} (GeV/c^{2})"));
  h_MKs->GetListOfFunctions()->Add(new TNamed("Contact" , "minakshi.nayak@wayne.edu"));
  h_MKs->GetListOfFunctions()->Add(new TNamed("Check", "M_{K_{S}} is expected to peak at 497 MeV/c^{2} with resolution around 5 MeV"));



  TH1F* h_Mpi0 = new TH1F("h_Mpi0","B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{0})K^{+}",100,0.096,0.164);
  //  ptree->Project("h_Mpi0", "B_D0_pi0_M", tmCuts);
  ptree->Project("h_Mpi0", "B_D0_pi0_M");
  h_Mpi0->GetXaxis()->SetTitle("M_{#pi^{0}} (GeV/c^{2})");
  h_Mpi0->GetYaxis()->SetTitle("Evts/bin");
  h_Mpi0->GetListOfFunctions()->Add(new TNamed("Description", "B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{0})K^{+}  M_{pi0} (GeV/c^{2})"));
  h_Mpi0->GetListOfFunctions()->Add(new TNamed("Contact" , "minakshi.nayak@wayne.edu"));
  h_Mpi0->GetListOfFunctions()->Add(new TNamed("Check", "M_{#pi^{0}} is expected to peak at 135 MeV/c^{2} with resolution around 8 MeV"));



  TH1F* h_MD = new TH1F("h_MD","B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{0})K^{+}",100,1.75,1.95);
  //  ptree->Project("h_MD", "B_D0_M", tmCuts);
  ptree->Project("h_MD", "B_D0_M");
  h_MD->GetXaxis()->SetTitle("M_{D} (GeV/c^{2})");
  h_MD->GetYaxis()->SetTitle("Evts/bin");
  h_MD->GetListOfFunctions()->Add(new TNamed("Description", "B^{+}#rightarrow D^{0}(K_{S}^{0} #pi^{0})K^{+}  M_{D} (GeV/c^{2})"));
  h_MD->GetListOfFunctions()->Add(new TNamed("Contact" , "minakshi.nayak@wayne.edu"));
  h_MD->GetListOfFunctions()->Add(new TNamed("Check", "M_{D} is expected to peak at 1.864 GeV/c^{2} with resolution around 27 MeV"));



  outputFile->cd();

  h_mbc->Write();
  h_deltae->Write();
  h_MKs->Write();
  h_Mpi0->Write();
  h_MD->Write();
}




void test2_1213012101(){

  TString inputfile("../1213012101.ntup.root");

  TFile* sample = new TFile(inputfile);
  TTree* tree = (TTree*)sample->Get("bdk");

  TFile* outputFile = new TFile("1213012101_Validation.root","RECREATE");

  plot(sample, tree, outputFile);


  outputFile->Close();

}
